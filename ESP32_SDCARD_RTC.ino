/*
Autor: James Ehlert Reinard.
Data: 09/04/2024.

Descrição:
Este projeto consiste em um sistema de monitoramento de temperatura e umidade com registro em um cartão SD e exibição em um display OLED.
O sistema utiliza um módulo RTC (Real-Time Clock) para registrar a data e hora das leituras.

Componentes:
ESP32: Microcontrolador utilizado como base do projeto.
Sensor AHT10: Sensor de temperatura e umidade.
Módulo RTC DS3231: Módulo de relógio em tempo real para manter o controle da data e hora.
Display OLED SSD1306: Display utilizado para exibir as informações.
Cartão SD: Utilizado para armazenar os dados das leituras.
Configuração:
Conectar os componentes conforme o esquemático.
Configurar o ambiente de desenvolvimento do Arduino IDE.
Instalar as bibliotecas necessárias:
Adafruit GFX Library
Adafruit SSD1306
AHTx
RTClib

Funcionamento:
O sistema inicializa o RTC, o sensor de temperatura e umidade, o display OLED e o cartão SD.
É gerado um nome único para o arquivo CSV com base na data e hora atuais.
O sistema realiza leituras periódicas da temperatura e umidade.
Os dados das leituras são registrados no arquivo CSV no formato: Código, Data, Hora, Temperatura (°C), Umidade (%).
As informações das leituras são exibidas no display OLED.
*/

#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <AHTxx.h>
#include <RTClib.h>

#define SCREEN_WIDTH 128 // Largura do display OLED, em pixels
#define SCREEN_HEIGHT 64 // Altura do display OLED, em pixels
#define AHT10_ADDRESS (0x38)

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
AHTxx aht10(AHT10_ADDRESS);
RTC_DS3231 rtc;
String filename; // Nome do arquivo para salvar os dados

void setup()
{
    Serial.begin(115200);

    if (!rtc.begin())
    {
        Serial.println("Falha ao iniciar o RTC!");
        while (1)
            ;
    }

    if (!SD.begin(5))
    {
        Serial.println("Falha ao montar o cartão SD");
        return;
    }

    if (!aht10.begin())
    {
        Serial.println("Falha ao inicializar o sensor AHT10");
        return;
    }

    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
    {
        Serial.println(F("Falha ao iniciar o display SSD1306"));
        for (;;)
            ;
    }

    if (rtc.lostPower())
    {
        Serial.println("RTC perdido poder! Reiniciando...");
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }

    // Gera o nome do arquivo com base na data e hora atual
    DateTime now = rtc.now();
    char filenameBuffer[30]; // Buffer para armazenar o nome do arquivo
    sprintf(filenameBuffer, "/data_%04d_%02d_%02d_%02d_%02d_%02d.csv",
            now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second());
    filename = filenameBuffer;

    // Abre o arquivo para escrita no início do programa
    File dataFile = SD.open(filename, FILE_WRITE);
    if (dataFile)
    {
        dataFile.println("Codigo,Data,Hora,Temp(C),Umidade(%)");
        dataFile.flush(); // Garante que todos os dados sejam gravados no cartão SD imediatamente
        dataFile.close();
    }
    else
    {
        Serial.println("Erro ao abrir o arquivo no cartão SD");
    }
}

void loop()
{
    DateTime now = rtc.now(); // Obter data/hora atual do RTC
    float temperatura = aht10.readTemperature();
    float umidade = aht10.readHumidity();

    if (isnan(temperatura) || isnan(umidade))
    {
        Serial.println("Falha ao ler os dados do sensor");
        return;
    }

    // Gera um código hexadecimal aleatório
    unsigned int randomNumber = random(0xFFFF);
    String hexCode = String(randomNumber, HEX);

    File dataFile = SD.open(filename, FILE_APPEND);
    if (dataFile)
    {
        // Grava os dados da leitura no arquivo CSV
        dataFile.print(hexCode); // Código único aleatório
        dataFile.print(",");
        dataFile.print(now.year(), DEC);
        dataFile.print("/");
        if (now.month() < 10) dataFile.print("0");
        dataFile.print(now.month(), DEC);
        dataFile.print("/");
        if (now.day() < 10) dataFile.print("0");
        dataFile.print(now.day(), DEC);
        dataFile.print(",");
        if (now.hour() < 10) dataFile.print("0");
        dataFile.print(now.hour(), DEC);
        dataFile.print(":");
        if (now.minute() < 10) dataFile.print("0");
        dataFile.print(now.minute(), DEC);
        dataFile.print(":");
        if (now.second() < 10) dataFile.print("0");
        dataFile.print(now.second(), DEC);
        dataFile.print(",");
        dataFile.print(temperatura);
        dataFile.print(",");
        dataFile.println(umidade);
        dataFile.flush(); // Garante que todos os dados sejam gravados no cartão SD imediatamente
        dataFile.close();
        Serial.println("Dados da leitura gravados com sucesso no cartão SD");
    }
    else
    {
        Serial.println("Erro ao abrir o arquivo no cartão SD");
    }

    // Exibe os dados do sensor e a data/hora atual no display OLED
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.print("Data: ");
    display.print(now.year(), DEC);
    display.print("/");
    display.print(now.month() < 10 ? "0" : "");
    display.print(now.month(), DEC);
    display.print("/");
    display.print(now.day() < 10 ? "0" : "");
    display.println(now.day(), DEC);
    display.print("Hora: ");
    display.print(now.hour() < 10 ? "0" : "");
    display.print(now.hour(), DEC);
    display.print(":");
    display.print(now.minute() < 10 ? "0" : "");
    display.print(now.minute(), DEC);
    display.print(":");
    display.print(now.second() < 10 ? "0" : "");
    display.print(now.second(), DEC);
    display.println(" ");
    display.println("Temp: " + String(temperatura) + " C");
    display.println("Umidade: " + String(umidade) + " %");
    display.display();
    delay(800); // Intervalo de 0.8 segundos entre as leituras
}
