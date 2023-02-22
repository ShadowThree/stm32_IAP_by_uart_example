#include <stdio.h>
#include <stdint.h>
// #include "./crc/crc.h"
#include "./ymodem/ymodem.h"
#include "windows.h"

#define COM_PORT        "COM6"
#define FIRMWARE_NAME   "./APP_red_LED.bin"

#define COM_READ_LEN    (90)

size_t fsize(FILE *fp);
HANDLE g_hCom;

int main(void)
{
    FILE *binFile;
    g_hCom = CreateFile(COM_PORT, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, 0);
    if (g_hCom == INVALID_HANDLE_VALUE)
    {
        printf("ERROR: COM port open failed!\n");
        exit(1);
    }

    // 设置读超时
    COMMTIMEOUTS timeouts;
    GetCommTimeouts(g_hCom, &timeouts);
    timeouts.ReadIntervalTimeout = 30;              // 3000
    timeouts.ReadTotalTimeoutMultiplier = 60;       // 6000
    timeouts.ReadTotalTimeoutConstant = 10000;       // 30000
    timeouts.WriteTotalTimeoutMultiplier = 20;
    timeouts.WriteTotalTimeoutConstant = 1000;
    SetCommTimeouts(g_hCom, &timeouts);

    // 设置串口配置信息
    DCB dcb;
    if (!GetCommState(g_hCom, &dcb))
    {
        printf("ERROR: GetCommState() failed\n");
        CloseHandle(g_hCom);
        exit(1);
    }
    int nBaud = 115200;
    dcb.DCBlength = sizeof(DCB);
    dcb.BaudRate = nBaud;      // 波特率为115200
    dcb.Parity = 0;            // 校验方式为无校验
    dcb.ByteSize = 8;          // 数据位为8位
    dcb.StopBits = ONESTOPBIT; // 停止位为1位
    if (!SetCommState(g_hCom, &dcb))
    {
        printf("ERROR: SetCommState() failed\n");
        CloseHandle(g_hCom);
        exit(1);
    }

    // 设置读写缓冲区大小
    static const int g_nZhenMax = 32768;
    if (!SetupComm(g_hCom, g_nZhenMax, g_nZhenMax))
    {
        printf("ERROR: SetupComm() failed\n");
        CloseHandle(g_hCom);
        exit(1);
    }

    // 清空缓冲
    PurgeComm(g_hCom, PURGE_RXCLEAR | PURGE_TXCLEAR);

    // 清除错误
    DWORD dwError;
    COMSTAT cs;
    if (!ClearCommError(g_hCom, &dwError, &cs))
    {
        printf("ERROR: ClearCommError() failed\n");
        CloseHandle(g_hCom);
        exit(1);
    }

    BOOL bErrorFlag = FALSE;
    uint8_t DataBuffer[] = {0x55, 0xAA, 0x06, 0x00, 0x01, 0x05, 0x00, 0x0B, 0xE1, 0x01, 0x35, 0x83};
    printf("\nsend UPDATE command: ");
    for (int i = 0; i < sizeof(DataBuffer); i++)
    {
        printf("D[%d]=[0x%02x] ", i, DataBuffer[i]);
    }
    printf("\n");

    DWORD dwBytesToWrite = sizeof(DataBuffer);
    DWORD dwBytesWritten = 0;
    bErrorFlag = WriteFile(
        g_hCom,          // open file handle
        DataBuffer,      // start of data to write
        dwBytesToWrite,  // number of bytes to write
        &dwBytesWritten, // number of bytes that were written
        NULL);           // no overlapped structure
    if (FALSE == bErrorFlag)
    {
        printf("Terminal failure: Unable to write to file.\n");
    }
    else
    {
        if (dwBytesWritten != dwBytesToWrite)
        {
            printf("Error: dwBytesWritten != dwBytesToWrite\n");
        }
        else
        {
            printf("Wrote %d bytes to test.txt successfully, wait ACK...\n\n", dwBytesWritten);
        }
    }

    // 读取串口数据
    uint8_t buf[COM_READ_LEN + 1] = {0};
    DWORD nLenOut = 0;
    if (ReadFile(g_hCom, buf, COM_READ_LEN, &nLenOut, NULL))
    {
        if (nLenOut) // 成功
        {
            printf("RecvLen = [%d]:\n  ", nLenOut);
            for (int i = 0; i < COM_READ_LEN; i++)
            {
                printf("D[%02d]=[0x%02x] ", i, buf[i]);
                if (i % 10 == 9)
                    printf("\n  ");
            }
            printf("\n");
        }
        else // 超时
        {
            printf("time out\n");
            CloseHandle(g_hCom);
        }
    }
    else
    {
        // 失败
        printf("ERROR: read() error!\n");
        CloseHandle(g_hCom);
    }

    printf("Prepare to send the bin file:\n");

    binFile = fopen(FIRMWARE_NAME, "rb");        // must "rb" for binary file!!!
    size_t fileLen = fsize(binFile);
    printf("  firmware FileSize = [%d]bytes\n", fileLen);
    char *fileContent = (char *)calloc(fileLen, sizeof(char));

    size_t readLen = 0;
    readLen = fread(fileContent, 1, fileLen, binFile); 
    if(readLen != fileLen) {
        printf("ERR: file read error, readLen[%d]\n", readLen);
    }

    Ymodem_Transmit(fileContent, "test.bin", fileLen);

    free(fileContent);
    fclose(binFile);
    // 关闭串口
    CloseHandle(g_hCom);

    // crcInit();

    return 0;
}

HAL_StatusTypeDef Serial_PutByte(char c)
{
    DWORD writeLen;
    // 设置写超时
    COMMTIMEOUTS timeouts;
    GetCommTimeouts(g_hCom, &timeouts);
    timeouts.WriteTotalTimeoutMultiplier = TX_TIMEOUT;
    timeouts.WriteTotalTimeoutConstant = 0;
    SetCommTimeouts(g_hCom, &timeouts);

    if(FALSE == WriteFile(g_hCom, &c, 1, &writeLen, NULL))
    {
        printf("Terminal failure: Unable to write to file.\n");
        CloseHandle(g_hCom);
        exit(1);
    }
    else
    {
        if (writeLen != 1)
        {
            printf("Error: dwBytesWritten != dwBytesToWrite\n");
            CloseHandle(g_hCom);
            exit(1);
        }
    }
    return HAL_OK;
}

HAL_StatusTypeDef send_pkg(char* c, uint32_t len, uint32_t timeout)
{
    DWORD writeLen;
    // 设置写超时
    COMMTIMEOUTS timeouts;
    GetCommTimeouts(g_hCom, &timeouts);
    timeouts.WriteTotalTimeoutMultiplier = 0;
    timeouts.WriteTotalTimeoutConstant = timeout;
    SetCommTimeouts(g_hCom, &timeouts);

    if(FALSE == WriteFile(g_hCom, c, len, &writeLen, NULL))
    {
        printf("Terminal failure: Unable to write to file.\n");
        CloseHandle(g_hCom);
        exit(1);
    }
    else
    {
        if (writeLen != len)
        {
            printf("Error: dwBytesWritten != dwBytesToWrite\n");
            CloseHandle(g_hCom);
            exit(1);
        }
    }
    return HAL_OK;
}

HAL_StatusTypeDef receive_byte(char* c, uint32_t timeout)
{
    DWORD readLen;
    // 设置读超时
    COMMTIMEOUTS timeouts;
    GetCommTimeouts(g_hCom, &timeouts);
    timeouts.ReadIntervalTimeout = 0;
    timeouts.ReadTotalTimeoutMultiplier = 0;
    timeouts.ReadTotalTimeoutConstant = timeout;
    SetCommTimeouts(g_hCom, &timeouts);

    if (ReadFile(g_hCom, c, 1, &readLen, NULL))
    {
        if (readLen != 1) // 超时
        {
            printf("time out\n");
            CloseHandle(g_hCom);
            exit(1);
        }
    }
    else
    {
        // 失败
        printf("ERROR: read() error!\n");
        CloseHandle(g_hCom);
        exit(1);
    }
    return HAL_OK;
}

void flush_uart_register(void)
{
    // 清空缓冲
    PurgeComm(g_hCom, PURGE_RXCLEAR | PURGE_TXCLEAR); 
}

/**
 * @brief   获取文件大小
 *
 * @param fp
 * @return long
 */
size_t fsize(FILE *fp)
{
    size_t n;
    fpos_t fpos;            // 当前位置
    fgetpos(fp, &fpos);     // 获取当前位置
    fseek(fp, 0, SEEK_END); // 定位文件指针到结尾
    n = ftell(fp);          // 返回文件指针到文件开头的字节数
    fsetpos(fp, &fpos);     // 恢复之前的位置
    return n;
}
