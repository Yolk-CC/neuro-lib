#include <Arduino.h>
#include <unity.h>
#include <NeuroLib.h>
#include <Stream.h>

#define TEST_BUFFER_SIZE 256

NeuroLib neuro;

class TestSerial : public Stream {
private:
    char buffer[TEST_BUFFER_SIZE];
    int writePos;
    int readPos;

public:
    TestSerial() : writePos(0), readPos(0) {
        memset(buffer, 0, TEST_BUFFER_SIZE);
    }

    void clear() {
        writePos = 0;
        readPos = 0;
        memset(buffer, 0, TEST_BUFFER_SIZE);
    }

    void writeString(const char* str) {
        int len = strlen(str);
        if (writePos + len < TEST_BUFFER_SIZE) {
            strcpy(&buffer[writePos], str);
            writePos += len;
        }
    }

    int available() override {
        return writePos - readPos;
    }

    int read() override {
        if (readPos < writePos) {
            return buffer[readPos++];
        }
        return -1;
    }

    int peek() override {
        if (readPos < writePos) {
            return buffer[readPos];
        }
        return -1;
    }

    void flush() override {
        clear();
    }

    size_t write(uint8_t) override {
        return 0;
    }
};

TestSerial testSerial;

void setUp(void) {
    testSerial.clear();
    neuro.begin(115200, testSerial);
}

void tearDown(void) {
    neuro.end();
}

void test_parse_csv_format() {
    testSerial.writeString("1.234,0,1000\n");
    
    TEST_ASSERT_TRUE(neuro.dataAvailable());
    
    EEGData data = neuro.readData();
    
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 1.234f, data.value);
    TEST_ASSERT_EQUAL(0, data.channel);
    TEST_ASSERT_GREATER_THAN(0, data.timestamp);
}

void test_parse_negative_values() {
    testSerial.writeString("-2.567,1,2000\n");
    
    EEGData data = neuro.readData();
    
    TEST_ASSERT_FLOAT_WITHIN(0.001f, -2.567f, data.value);
    TEST_ASSERT_EQUAL(1, data.channel);
}

void test_parse_scientific_notation() {
    testSerial.writeString("1.5e-3,0,3000\n");
    
    EEGData data = neuro.readData();
    
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0015f, data.value);
}

void test_parse_multiple_channels() {
    testSerial.writeString("0.5,0,4000\n");
    EEGData data1 = neuro.readData();
    TEST_ASSERT_EQUAL(0, data1.channel);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.5f, data1.value);
    
    testSerial.writeString("0.8,1,4001\n");
    EEGData data2 = neuro.readData();
    TEST_ASSERT_EQUAL(1, data2.channel);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.8f, data2.value);
    
    testSerial.writeString("1.2,2,4002\n");
    EEGData data3 = neuro.readData();
    TEST_ASSERT_EQUAL(2, data3.channel);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 1.2f, data3.value);
}

void test_parse_with_spaces() {
    testSerial.writeString("  1.234 , 0 , 1000  \n");
    
    EEGData data = neuro.readData();
    
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 1.234f, data.value);
}

void test_parse_empty_lines() {
    testSerial.writeString("\n");
    testSerial.writeString("   \n");
    testSerial.writeString("1.5,0,5000\n");
    
    EEGData data = neuro.readData();
    
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 1.5f, data.value);
}

void test_parse_malformed_data() {
    testSerial.writeString("invalid\n");
    EEGData data1 = neuro.readData();
    
    testSerial.writeString("1.0\n");
    EEGData data2 = neuro.readData();
    
    testSerial.writeString("1.0,2\n");
    EEGData data3 = neuro.readData();
}

void test_parse_custom_delimiter() {
    neuro.setDelimiter(';');
    
    testSerial.writeString("2.5;1;6000\n");
    
    EEGData data = neuro.readData();
    
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 2.5f, data.value);
    TEST_ASSERT_EQUAL(1, data.channel);
    
    neuro.setDelimiter(',');
}

void test_parse_tab_delimiter() {
    neuro.setDelimiter('\t');
    
    testSerial.writeString("3.14\t2\t7000\n");
    
    EEGData data = neuro.readData();
    
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 3.14f, data.value);
    TEST_ASSERT_EQUAL(2, data.channel);
    
    neuro.setDelimiter(',');
}

void test_read_channel_buffer() {
    for (int i = 0; i < 10; i++) {
        char line[50];
        sprintf(line, "%.3f,0,%d\n", 0.1f * i, 8000 + i);
        testSerial.writeString(line);
    }
    
    float buffer[10];
    int count = neuro.readChannel(0, buffer, 10);
    
    TEST_ASSERT_EQUAL(10, count);
    
    for (int i = 0; i < 10; i++) {
        TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.1f * i, buffer[i]);
    }
}

void test_read_channel_filtering() {
    testSerial.writeString("1.0,0,9000\n");
    testSerial.writeString("2.0,1,9001\n");
    testSerial.writeString("3.0,0,9002\n");
    testSerial.writeString("4.0,1,9003\n");
    testSerial.writeString("5.0,0,9004\n");
    
    float buffer[5];
    int count = neuro.readChannel(0, buffer, 5);
    
    TEST_ASSERT_EQUAL(3, count);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 1.0f, buffer[0]);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 3.0f, buffer[1]);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 5.0f, buffer[2]);
}

void test_parse_high_frequency_data() {
    for (int i = 0; i < 20; i++) {
        char line[50];
        float value = sinf(i * 0.1f) * 10.0f;
        sprintf(line, "%.4f,0,%d\n", value, 10000 + i);
        testSerial.writeString(line);
    }
    
    float buffer[20];
    int count = neuro.readChannel(0, buffer, 20);
    
    TEST_ASSERT_EQUAL(20, count);
    
    for (int i = 0; i < 20; i++) {
        float expected = sinf(i * 0.1f) * 10.0f;
        TEST_ASSERT_FLOAT_WITHIN(0.01f, expected, buffer[i]);
    }
}

void test_parse_large_values() {
    testSerial.writeString("999999.99,0,11000\n");
    testSerial.writeString("-999999.99,1,11001\n");
    
    EEGData data1 = neuro.readData();
    EEGData data2 = neuro.readData();
    
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 999999.99f, data1.value);
    TEST_ASSERT_FLOAT_WITHIN(1.0f, -999999.99f, data2.value);
}

void test_parse_zero_values() {
    testSerial.writeString("0.0,0,12000\n");
    testSerial.writeString("0,1,12001\n");
    testSerial.writeString("0.000,2,12002\n");
    
    EEGData data1 = neuro.readData();
    EEGData data2 = neuro.readData();
    EEGData data3 = neuro.readData();
    
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, data1.value);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, data2.value);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, data3.value);
}

void setup() {
    delay(2000);
    UNITY_BEGIN();
    
    RUN_TEST(test_parse_csv_format);
    RUN_TEST(test_parse_negative_values);
    RUN_TEST(test_parse_scientific_notation);
    RUN_TEST(test_parse_multiple_channels);
    RUN_TEST(test_parse_with_spaces);
    RUN_TEST(test_parse_empty_lines);
    RUN_TEST(test_parse_malformed_data);
    RUN_TEST(test_parse_custom_delimiter);
    RUN_TEST(test_parse_tab_delimiter);
    RUN_TEST(test_read_channel_buffer);
    RUN_TEST(test_read_channel_filtering);
    RUN_TEST(test_parse_high_frequency_data);
    RUN_TEST(test_parse_large_values);
    RUN_TEST(test_parse_zero_values);
    
    UNITY_END();
}

void loop() {
}
