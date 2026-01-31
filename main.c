#include<stdio.h>
#include<stdlib.h>
#define maxlan 256
#define max_peripheral 50
#define fileName "device_init.h"

enum PeriphType { GPIO_DEV, I2C_DEV, SPI_DEV, UART_DEV };

enum PinMode { INPUT, OUTPUT, ALTERNATE };

typedef struct {
    unsigned int pin;
    enum PinMode mode;
    unsigned int pull; 
} GPIOConfig;

typedef struct {
    unsigned int addr;
    unsigned int speed; 
} I2CConfig;

typedef struct {
    unsigned int cs;
    unsigned int clock;
    unsigned int mode;
} SPIConfig;

typedef struct {
    unsigned int tx;
    unsigned int rx;
    unsigned int baud;
} UARTConfig;

typedef union {
    GPIOConfig gpio;
    I2CConfig i2c;
    SPIConfig spi;
    UARTConfig uart;
} PeriphConfig;

typedef struct {
    enum PeriphType type;
    char name[32];
    PeriphConfig config;
    unsigned int enabled : 1;
} Peripheral;

static int parsed_peripheral_count = 0;
static int parsed_gpio_count = 0;
static int used_gpio_pins = 0;
static int parsed_i2c_count = 0;
static int parsed_spi_count = 0;
static int parsed_uart_count = 0;
static int no_of_conflicts = 0;
static int disabled = 0;

void parse_gpio_line(char buffer[], Peripheral *peripheral) {
    peripheral->type = GPIO_DEV;
    peripheral->enabled = 1;
    unsigned int i=0;
    while(buffer[i]!=':')
    {
        peripheral->name[i]=buffer[i];
        i++;
    }
    peripheral->name[i]='\0';
    i++;
    while(buffer[i]!='\0')
    {
        if(buffer[i]=='p' && buffer[i+1]=='i' && buffer[i+2]=='n')
        {
            i+=4;
            unsigned int num=0;
            while(buffer[i]!=',' && buffer[i]!='\0')
            {
                num *= 10;
                num += (buffer[i]-'0');
                i++;
            }
            peripheral->config.gpio.pin = num;
        }
        else if(buffer[i]=='m' && buffer[i+1]=='o' && buffer[i+2]=='d' && buffer[i+3]=='e')
        {
            i+=5;
            if(buffer[i]=='I' && buffer[i+1]=='N')
            {
                peripheral->config.gpio.mode = INPUT;
            }
            else if(buffer[i]=='O' && buffer[i+1]=='U')
            {
                peripheral->config.gpio.mode = OUTPUT;
            }
            else if(buffer[i]=='A' && buffer[i+1]=='L')
            {
                peripheral->config.gpio.mode = ALTERNATE;
            }
            while(buffer[i]!=',' && buffer[i]!='\0')
            {
                i++;
            }
        }
        else if(buffer[i]=='p')
        {
            i+=5;
            if(buffer[i]=='U')
            {
                peripheral->config.gpio.pull = 1;
            }
            else if(buffer[i]=='D')
            {
                peripheral->config.gpio.pull = 2;
            }
            else
            {
                peripheral->config.gpio.pull = 0;
            }
            while(buffer[i]!=',' && buffer[i]!='\0')
            {
                i++;
            }
        }
        else if(buffer[i]=='e' && buffer[i+1]=='n' && buffer[i+2]=='a' && buffer[i+3]=='b')
        {
            i+=8;
            if(buffer[i]=='f')
            {
                peripheral->enabled = 0;
            }
        }
        i++;
    }
    // printf("Name: %s  ", peripheral->name);
    // printf("Pin: %d  ", peripheral->config.gpio.pin);
    // printf("Mode: %d  ", peripheral->config.gpio.mode);
    // printf("Pull: %d  ", peripheral->config.gpio.pull);
    // printf("Enabled: %d\n", peripheral->enabled);
}

void parse_i2c_line(char buffer[], Peripheral *peripheral) {
    peripheral->type = I2C_DEV;
    peripheral->enabled = 1;
    int i=0;
    while(buffer[i]!=':')
    {
        peripheral->name[i]=buffer[i];
        i++;
    }
    peripheral->name[i]='\0';
    i++;
    while(buffer[i]!='\0')
    {
        if(buffer[i]=='a' && buffer[i+1]=='d')
        {
            i+=5;
            char temp[4];
            int idx = 0;
            while(buffer[i]!=',' && buffer[i]!='\0')
            {
                temp[idx]=buffer[i];
                i++;
                idx++;
            }
            unsigned long ul = strtoul(temp, NULL, 0);
            unsigned int ui = (unsigned int)(ul);
            peripheral->config.i2c.addr = ui;
        }
        else if(buffer[i]=='s' && buffer[i+1]=='p' && buffer[i+2]=='e')
        {
            i+=6;
            unsigned int num=0;
            while(buffer[i+1]!=',' && buffer[i+1]!='\0')
            {
                num *= 10;
                num += (buffer[i]-'0');
                i++;
            }
            peripheral->config.i2c.speed = num;
        }
        else if(buffer[i]=='e' && buffer[i+1]=='n' && buffer[i+2]=='a' && buffer[i+3]=='b')
        {
            i+=8;
            if(buffer[i]=='f')
            {
                peripheral->enabled = 0;
            }
        }
        i++;
    }
    // printf("Name: %s  ", peripheral->name);
    // printf("Addr: 0x%X  ", peripheral->config.i2c.addr);
    // printf("Speed: %d  ", peripheral->config.i2c.speed);
    // printf("Enabled: %d\n", peripheral->enabled);
}

void parse_spi_line(char buffer[], Peripheral *peripheral) {
    peripheral->type = SPI_DEV;
    peripheral->enabled = 1;
    int i=0;
    while(buffer[i]!=':')
    {
        peripheral->name[i]=buffer[i];
        i++;
    }
    peripheral->name[i]='\0';
    i++;
    while(buffer[i]!='\0')
    {
        if(buffer[i]=='c' && buffer[i+1]=='s')
        {
            i+=3;
            unsigned int num=0;
            while(buffer[i]!=',' && buffer[i]!='\0')
            {
                num *= 10;
                num += (buffer[i]-'0');
                i++;
            }
            peripheral->config.spi.cs = num;
        }
        else if(buffer[i]=='c' && buffer[i+1]=='l' && buffer[i+2]=='o')
        {
            i+=6;
            unsigned int num=0;
            while(buffer[i]!=',' && buffer[i]!='\0')
            {
                num *= 10;
                num += (buffer[i]-'0');
                i++;
            }
            peripheral->config.spi.clock = num;
        }
        else if(buffer[i]=='m' && buffer[i+1]=='o' && buffer[i+2]=='d')
        {
            i+=5;
            peripheral->config.spi.mode = (buffer[i]-'0');
        }
        else if(buffer[i]=='e' && buffer[i+1]=='n' && buffer[i+2]=='a' && buffer[i+3]=='b')
        {
            i+=8;
            if(buffer[i]=='f')
            {
                peripheral->enabled = 0;
            }
        }
        i++;
    }
    // printf("Name: %s  ", peripheral->name);
    // printf("CS: %d  ", peripheral->config.spi.cs);
    // printf("Clock: %d  ", peripheral->config.spi.clock);
    // printf("Mode: %d  ", peripheral->config.spi.mode);
    // printf("Enabled: %d\n", peripheral->enabled);
}

void parse_uart_line(char buffer[], Peripheral *peripheral) {
    peripheral->type = UART_DEV;
    peripheral->enabled = 1;
    int i=0;
    while(buffer[i]!=':')
    {
        peripheral->name[i]=buffer[i];
        i++;
    }
    peripheral->name[i]='\0';
    i++;
    while(buffer[i]!='\0')
    {
        if(buffer[i]=='t' && buffer[i+1]=='x')
        {
            i+=3;
            unsigned int num=0;
            while(buffer[i]!=',' && buffer[i]!='\0')
            {
                num *= 10;
                num += (buffer[i]-'0');
                i++;
            }
            peripheral->config.uart.tx = num;
        }
        else if(buffer[i]=='r' && buffer[i+1]=='x')
        {
            i+=3;
            unsigned int num=0;
            while(buffer[i]!=',' && buffer[i]!='\0')
            {
                num *= 10;
                num += (buffer[i]-'0');
                i++;
            }
            peripheral->config.uart.rx = num;
        }
        else if(buffer[i]=='b' && buffer[i+1]=='a')
        {
            i+=5;
            unsigned int num=0;
            while(buffer[i+1]!=',' && buffer[i+1]!='\0')
            {
                num *= 10;
                num += (buffer[i]-'0');
                i++;
            }
            peripheral->config.uart.baud = num;
        }
        else if(buffer[i]=='e' && buffer[i+1]=='n' && buffer[i+2]=='a' && buffer[i+3]=='b')
        {
            i+=8;
            if(buffer[i]=='f')
            {
                peripheral->enabled = 0;
            }
        }
        i++;
    }
    // printf("Name: %s  ", peripheral->name);
    // printf("tx: %d  ", peripheral->config.uart.tx);
    // printf("rx: %d  ", peripheral->config.uart.rx);
    // printf("baud: %d  ", peripheral->config.uart.baud);
    // printf("Enabled: %d\n", peripheral->enabled);
}

Peripheral* parse_config_file(FILE* filePtr) {
    printf("Parsing configuration file...\n");
    char buffer[maxlan];
    int total_peripheral = 0;
    Peripheral *peripheral = (Peripheral*)malloc(sizeof(Peripheral)*max_peripheral);
    while(fgets(buffer, maxlan, filePtr)!=NULL)
    {
        if(buffer[0]=='G')
        {
            total_peripheral++;
            parsed_gpio_count++;
            parse_gpio_line(buffer, &peripheral[total_peripheral-1]);
        }
        else if(buffer[0]=='I')
        {
            total_peripheral++;
            parsed_i2c_count++;
            parse_i2c_line(buffer, &peripheral[total_peripheral-1]);
        }
        else if(buffer[0]=='S')
        {
            total_peripheral++;
            parsed_spi_count++;
            parse_spi_line(buffer, &peripheral[total_peripheral-1]);
        }
        else if(buffer[0]=='U')
        {
            total_peripheral++;
            parsed_uart_count++;
            parse_uart_line(buffer, &peripheral[total_peripheral-1]);
        }
    }
    printf("Parsed %d peripherals\n", total_peripheral);
    printf("\n");
    parsed_peripheral_count = total_peripheral;
    return peripheral;
}

void validate_gpio(Peripheral* peripheral) {
    unsigned int curr_pin = peripheral->config.gpio.pin;
    if(curr_pin<0 || curr_pin>31) 
    {
        no_of_conflicts++;
        printf("ERROR: Invalid Pin Number %d\nGPIO Validation Failed for %s\n", curr_pin, peripheral->name);
    } 
    static unsigned int used_pins = 0;
    if(used_pins & (1 << curr_pin)) 
    {
        no_of_conflicts++;
        printf("ERROR: GPIO pin %d already allocated\nValidation failed for %s\n", curr_pin, peripheral->name);
    }
    else
    {
        used_gpio_pins++;
        used_pins |= (1 << curr_pin);
    }
}

void validate_i2c(Peripheral* peripheral) {
    int curr_addr = peripheral->config.i2c.addr;
    if(curr_addr<8 || curr_addr>127)
    {
        no_of_conflicts++;
        printf("ERROR: Invalid Address 0x%X\nI2C Validation Failed for %s\n", curr_addr, peripheral->name);
    }
    static unsigned int used_addresses[120] = {0};
    if(used_addresses[curr_addr-8] == 1) 
    {
        no_of_conflicts++;
        printf("ERROR: I2C address 0x%X already allocated\nValidation failed for %s\n", curr_addr, peripheral->name);
    }
    used_addresses[curr_addr-8] = 1;
}

void validate_spi(Peripheral* peripheral) {
    int curr_cs = peripheral->config.spi.cs;
    if(curr_cs<0 || curr_cs>31)
    {
        no_of_conflicts++;
        printf("ERROR: Invalid Chip Select %d\nSPI Validation Failed for %s\n", curr_cs, peripheral->name);
    }
    static unsigned int used_cs = 0;
    if(used_cs & (1 << curr_cs)) 
    {
        no_of_conflicts++;
        printf("ERROR: SPI Chip Select %d already allocated\nValidation failed for %s\n", curr_cs, peripheral->name);
    }
    used_cs |= (1 << curr_cs);
}

void validate_uart(Peripheral* peripheral) {
    int curr_tx = peripheral->config.uart.tx;
    int curr_rx = peripheral->config.uart.rx;
    if(curr_tx<0 || curr_tx>31 || curr_rx<0 || curr_rx>31)
    {
        no_of_conflicts++;
        printf("ERROR: Invalid TX or RX Pin\nUART Validation Failed for %s\n", peripheral->name);
    }
    static unsigned int used_pins = 0;
    if(used_pins & (1 << curr_tx)) 
    {
        no_of_conflicts++;
        printf("ERROR: UART TX pin %d already allocated\nValidation failed for %s\n", curr_tx, peripheral->name);
    }
    if(used_pins & (1 << curr_rx)) 
    {
        no_of_conflicts++;
        printf("ERROR: UART RX pin %d already allocated\nValidation failed for %s\n", curr_rx, peripheral->name);
    }
    used_pins |= (1 << curr_tx);
    used_pins |= (1 << curr_rx);
}

void validate_peripheral(Peripheral* peripheral) {
    printf("Validating peripherals...\n");
    for(int i=0;i<parsed_peripheral_count;i++)
    {
        Peripheral* peri = &peripheral[i];
        if(peri->enabled == 0)
        {
            disabled++;
            continue;
        }
        if(peri->type == GPIO_DEV)
        {
            validate_gpio(peri);
        }
        else if(peri->type == I2C_DEV)
        {
            validate_i2c(peri);
        }
        else if(peri->type == SPI_DEV)
        {
            validate_spi(peri);
        }
        else
        {
            validate_uart(peri);
        }
    }
}

void printf_gpio(FILE* filePtr, Peripheral* peripheral) {
    for(int i=0;i<parsed_peripheral_count;i++)
    {
        Peripheral* peri = &peripheral[i];
        if(peri->enabled == 0)
        {
            continue;
        }
        if(peri->type == GPIO_DEV)
        {
            char line[250];
            sprintf(line, "// GPIO: %s\n#define %s_PIN %d\n#define %s_MODE %d\n#define %s_PULL %d\n\n", peri->name, peri->name, peri->config.gpio.pin, peri->name, peri->config.gpio.mode, peri->name, peri->config.gpio.pull);
            fputs(line, filePtr);
        }
    }
}

void printf_i2c(FILE* filePtr, Peripheral* peripheral) {
    for(int i=0;i<parsed_peripheral_count;i++)
    {
        Peripheral* peri = &peripheral[i];
        if(peri->enabled == 0)
        {
            continue;
        }
        if(peri->type == I2C_DEV)
        {
            char line[250];
            sprintf(line, "// I2C: %s\n#define %s_ADDR 0x%X\n#define %s_SPEED %d\n\n", peri->name, peri->name, peri->config.i2c.addr, peri->name, peri->config.i2c.speed);
            fputs(line, filePtr);
        }
    }
}

void printf_spi(FILE* filePtr, Peripheral* peripheral) {
    for(int i=0;i<parsed_peripheral_count;i++)
    {
        Peripheral* peri = &peripheral[i];
        if(peri->enabled == 0)
        {
            continue;
        }
        if(peri->type == SPI_DEV)
        {
            char line[250];
            sprintf(line, "// SPI: %s\n#define %s_CS %d\n#define %s_CLOCK %d\n#define %s_MODE %d\n\n", peri->name, peri->name, peri->config.spi.cs, peri->name, peri->config.spi.clock, peri->name, peri->config.spi.mode);
            fputs(line, filePtr);
        }
    }
}

void printf_uart(FILE* filePtr, Peripheral* peripheral) {
    for(int i=0;i<parsed_peripheral_count;i++)
    {
        Peripheral* peri = &peripheral[i];
        if(peri->enabled == 0)
        {
            continue;
        }
        if(peri->type == UART_DEV)
        {
            char line[250];
            sprintf(line, "// UART: %s\n#define %s_TX %d\n#define %s_RX %d\n#define %s_BAUD %d\n\n", peri->name, peri->name, peri->config.uart.tx, peri->name, peri->config.uart.rx, peri->name, peri->config.uart.baud);
            fputs(line, filePtr);
        }
    }
}

unsigned char calculate_checksum(Peripheral* peripheral) {
    unsigned char checksum = 0;
    unsigned char *temp = (unsigned char *)peripheral;
    for(int i=0;i<parsed_peripheral_count;i++)
    {
        checksum ^= temp[i];
    }
    return checksum;
}

void generate_output_file(Peripheral* peripheral) {
    FILE* filePtr = fopen(fileName, "w");
    if(filePtr == NULL)
    {
        printf("Can't open file\n");
        return;
    }
    fputs("/* Auto-generated device initialization header */\n/* Generated from device configuration */\n\n", filePtr);
    fputs("#ifndef DEVICE_INIT_H\n#define DEVICE_INIT_H\n\n", filePtr);
    printf_gpio(filePtr, peripheral);
    printf_i2c(filePtr, peripheral);
    printf_spi(filePtr, peripheral);
    printf_uart(filePtr, peripheral);
    unsigned char checksum = calculate_checksum(peripheral);
    fprintf(filePtr, "// Configuration checksum\n#define CONFIG_CHECKSUM 0x%X\n\n", checksum);
    fputs("#endif // DEVICE_INIT_H\n\n", filePtr);
    fclose(filePtr);
}

int main()
{
    FILE* filePtr = fopen("device_config_file.txt", "r");
    if(filePtr == NULL)
    {
        printf("Can't open file\n");
        return -1;
    }
    printf("=== Device Configuration Parser ===\n");
    printf("\n");
    Peripheral* pptr = parse_config_file(filePtr);
    validate_peripheral(pptr);
    if(no_of_conflicts == 0)
    {
        printf("All peripherals validated successfully!\n");
        printf("\n");
        printf("=== Configuration Statistics ===\n");
        printf("Total Peripherals: %d\n", parsed_peripheral_count);
        printf("  GPIO devices: %d\n", parsed_gpio_count);
        printf("  I2C devices: %d\n", parsed_i2c_count);
        printf("  SPI devices: %d\n", parsed_spi_count);
        printf("  UART devices: %d\n", parsed_uart_count);
        printf("GPIO pins allocated: %d/32\n", used_gpio_pins);
        printf("Enabled peripherals: %d\n", parsed_peripheral_count-disabled);
        printf("Disabled peripherals: %d\n", disabled);
        unsigned char checksum = calculate_checksum(pptr);
        printf("Configuration checksum: 0x%X\n", checksum);
        printf("================================\n");
        printf("\n");
        generate_output_file(pptr);
        printf("Generated output file: device_init.h\n");
        printf("\n");
        printf("Configuration processing completed successfully!\n");
        printf("\n");
    }
    else
    {
        printf("\nConfiguration validation failed\n");
        printf("\n");
    }
    fclose(filePtr);
    return 0;
}