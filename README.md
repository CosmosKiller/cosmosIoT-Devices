# :zap: :bulb: CosmosIoT Devices :satellite: :electric_plug:

> Here you'll find the code for all of the devices manufactured @CosmosIoT 

## Before you start
* It's crucial that before actually start working on any project, you know and apply the conventions regarding code writing style and formatting, to ensure your code is consistent with the codebase of the platform. 

### C style formatting
* Here, we apply the classic c-style formatting, following the guidelines proposed in the second edition of 'The C Programming Language' and the LLVM style. Which includes, break before braces only when defining functions, alignment of consecutive macros and an indent width of 4 spaces, among others parameters.
* To guarantee that the formatting style, will be the same as the described before, yo can copy the following line in the **Clang_format_fallback Style** option from the C/C++ extension for VS Code:
 ```js
 { BasedOnStyle: LLVM, UseTab: Never, IndentWidth: 4, TabWidth: 4, BreakBeforeBraces: Linux, AllowShortIfStatementsOnASingleLine: false, IndentCaseLabels: false, ColumnLimit: 0, AccessModifierOffset: -4, NamespaceIndentation: All, FixNamespaceComments: false, AlignConsecutiveMacros: true }
 ```

> Press [Ctrl+,] (Linux/Win) or [Command+,] to open VS Code settings and then type *Clang_format_fallback Style* in the browse bar. Then paste the line described above.
>
> You should end up with something similar to the picture below.

<img src='https://i.imgur.com/NP5YZ5A.png'>

* Also, make sure you have checked the options for formatting on paste, save and type. This will ensure your code will be correctly styled.
> Press [Ctrl+,] (Linux/Win) or [Command+,] to open VS Code settings and then type *format* in the browse bar. Then check all three boxes, as shown below.

<img src='https://i.imgur.com/gLY0Gpu.png'>

### Macros, Variables, Derived data types and Functions naming
* As a general rule, when naming variables/functions we use `snake_case`, if the name consists of more than one word. E.g.:
```C
int this_is_my_variable;
```
* Only when declaring pointer variables we use `camelCase`. The name of the pointer must start with the letter `p`. E.g.:
```C
const char *pMyPointer;
```
* Same rules that apply for varibles/funtions, also applies when defining new derivade data types (structs/enums). When declaring a new typedef struct, be sure to append `_t` at the end of the name. Similarly, when declaring a typedef enum append `_e` at the end of the name; Also, the members of the enums must be declared using `UPPER_SNAKE_CASE`. E.g.:
```C
typedef struct {
	int my_int;
	unsigned int my_uint;
	char my_char;
	float my_float_array[];
} new_struct_t;

typedef enum {
	VALUE_0 = 0,
	VALUE_1,
	VALUE_2,
	VALUE_3,
	VALUE_4,
	VALUE_5,
} new_enum_e;

```
* Unlike variables/functions, Macros, must always be declared using `UPPER_CAMEL_CASE`, if the name consists of multiple words. E.g.:
```C
#define OBJECT_AS_MARCO "My Macro"

#define FUNCTION_AS_MACRO(a, b) ((a+b)/(a*b))
```
### Comments
* Use `//` for single line comments. For multi-line comments  use a `/* */` block ; Try to keep each line even.
```C
// This is a single line comment
void my_func(void)
{
    do_something();
}

/*
 * This is a multi-line comment.
 * Try to keep all the lines as
 * even as posible.
 */
void other_func(void)
{
    do_something_else();
}
```
### Documenting Code
* Documenting is as important as writing good and clean code. It'll help us to easily understand how to make propper usage of the functionalities in the source/header files.
* In this matter, when documenting your code please be sure follow [Doxygen style](https://www.doxygen.nl/). You are doing it by inserting special commands, for instance `@param`, into standard comments blocks, I.g.:
```C
/**
 * @brief This function is used to controll the state (on/off)
 * of a certain socket
 *
 * @param sn_value Serial number used to compare and
 * determine with which socket we are trying to interact
 * @param qty Quantity of sockets used in the project
 * @param pSocket Pointer to the strutct that contains all of the info
 * about the sockets used in the project
 */
void cosmos_socket_control(const char sn_value[15], int qty, cosmos_devices_t *pSocket);
```
* When documenting a `#define` as well as members of a `struct` or `enum`, comments should be done using `/*!< */`.
```C
#define COSMOS_MAP(x, in_min, in_max, out_min, out_max) ((x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min) /*!< Arduino style map function */

/**
 * @brief Standard structure for devices management
 * DON'T MODIFY THIS STRUCT!
 */
typedef struct {
    const char sn[15]; /*!< Serial Number of the devices */
    const int pin[3];  /*!< Pins in which the device will be connected | For LED -> {rPin, gPin, bPin} | For other -> {devPin, 0, 0} */
    int state;         /*<! Initial state of the device (0: low ; 1: high) */
} cosmos_devices_t;

```
### Coding style sample
* Below you will see a code sample of all the points discussed above
```c
#ifndef MAIN_COSMOS_PUMP_H_
#define MAIN_COSMOS_PUMP_H_

#include "driver/adc.h"

#include "cosmos_devices.h"

#define NO_OF_SAMPLES 64 /*!< Standar sample rate for ADC multisampling */

#define INT_TO_ADC_CHANNEL(snr_pin) (snr_pin > 35 ? snr_pin - 36 : snr_pin - 28) /*!< Because the GPIO pin is expresed as an int, we need to convert it to an adc1_channel_t */

/**
 * @brief States of the pump
 *
 */
typedef enum {
    ENGAGE_OK = 0, /*!< Pump started */
    ENGAGE_RN,     /*!< Pump running */
    ENGAGE_NO,     /*!< Pump in standby */
} cosmos_pump_state_e;

/**
 * @brief Types of sensor
 *
 */
typedef enum {
    SNR_TYPE_TH = 0, /*!< Temperature sensor */
    SNR_TYPE_WL,     /*!< Water level sensor */
    SNR_TYPE_SM,     /*!< Soil moisture sensor */
    SNR_TYPE_HM,     /*!< Hunidity sensor */
} cosmos_pump_snr_type_e;

/**
 * @brief Use this struct within the
 * cosmos_pump_t to store the parameters
 * of the sensors
 */
typedef struct {
    const int thr_max;          /*!< Max threshold Level */
    const int thr_min;          /*!< Min threshold Level */
    const int pin_num;          /*!< Pin number in which the sensor is connected */
    cosmos_devices_type_e type; /*!< Sensor type */
} cosmos_pump_snr_info_t;

/**
 * @brief Use this struct to define all the parameters
 * that needs to be checked in order to get the pump
 * going
 *
 * @note If you are going to do without any of the sensors,
 * just create an arry of zeros -> {0, 0}
 */
typedef struct {
    cosmos_devices_t *pPumpInfo;      /*<! Pump relay serial number, pins and state */
    const int *pLedInfo;              /*!< Pins in which the pump's LED is connected */
    cosmos_pump_snr_info_t *pSnrInfo; /*!< Sensor information */
} cosmos_pump_t;

/**
 * @brief Struct in which the sensed data will be stored
 *
 */
typedef struct {
    int wl_data;
    int sm_data;
    float tm_data;
    float hm_data;
} cosmos_snr_data_t;

/**
 * @brief Call this function whenever you want to check the
 * status of the pump
 *
 * @param pTopic Incoming topic from the MQTT broker
 * @param pPump Pointer to the struct that defines the pump
 * @param engage
 *          - If you want to turn on the pump ENGAGE_OK ;
 *          - If you want to leave it in idle ENGAGE_NO ;
 *          - If pump is already running ENGAGE_RN ;
 * @param pSensorData Pointer to the struct in which the sensed data will be stored
 */
void cosmos_pump_control(char *pTopic, cosmos_pump_t *pPump, int engage, cosmos_snr_data_t *pSensorData);

#endif /* MAIN_COSMOS_PUMP_H_ */
```
---

## Getting started
* First of all, you'll need to install the PlatformIO Core (CLI) :ant:
* On a terminal you can simply type `pip install platformio` to get the client globally installed
* Now you are able to use the `pio` command. For more information, type `pio -h` 
### Working on a existent project
* Navigate to the project's folder you want to work on and find the *src* folder inside of it
* Open the *main.c* inside the *src* folder
* Start coding! :computer:
* If you want to build/test/upload you code, type `pio run -e [theNameOfTheProjectYouAreWorkingOn]`. For more information, type `pio run -h`
### Creating a new project
* First you'll need to create the project's folder where the *main.c* and the *platformio.ini* files will live
* Be sure to be as explicit and illustrative as possible when naming your project/folder ; e.g., 'automatedIrrigationSystem'
* Once you've created the project directory, you'll need to create the *platformio.ini* file. This file, is used as project's configuration file that PlatformIO will use to build/upload/test your source code.
* This file **must** include the following section (denoted by the `[header]` 'platformio')
```ini
[platformio]
lib_dir = ../.commonFiles/lib
include_dir = ./tasks
```
* This header will make a 'global directory' (named '.commonFiles') where all you libdeps, custom libs and include files will be stored, so you use them in all of your projects without the need of downloading them over and over.
* You can configure the rest of the sections according to what the project needs. Here's an example for a project based of a ESP-WROOM-32E module, which includes a library for a BME680 sensor.
```ini
[env:lilFlowerPal]
platform = espressif32
board = esp32doit-devkit-v1
framework = espidf
build_flags =
    -D CONFIG_I2CDEV_TIMEOUT=1000
lib_extra_dirs = 
    /home/cosmoskiller/.platformio/packages/framework-espidf/components/esp-idf-lib/components
lib_ldf_mode = chain+
upload_speed = 250000
```
* Next step, is to create the *src* folder and the *main.c* file inside of it.
* Finally is time to get your hands dirty and start coding! :keyboard:
> Below you can see a typical project structure based on the ESP-IDF framework. In the fisrt example (AKA 'project1') you can see a project that requires one extra task aside the entry point (main.c) and it also requeires a third-party dependencie (E.g., the [DHT driver from UnlceRus](https://github.com/UncleRus/esp-idf-lib/tree/master/components/dht)). The second example has a much more simple folder structure as, it only contains project's entry point.
```sequence
globalDir/
|--.commonFiles/
|  |--lib/ ---> Libraries that are common to many projects.
|	
|--project1/
|  |--.pio/
|  |  |--build/
|  |  |
|  |  |--libdeps/ ---> Third-party dependencies.
|  |  |  |
|  |  |  |--project1/
|  |  |     |
|  |  |     |--custom_lib/
|  |  |        |--custom_lib.h
|  |  |        |--custom_lib.c
|  |
|  |--src/ ---> Project's entry point and tasks source code.
|  |  |--main.c
|  |  |--my_library.c
|  |
|  |--tasks/ ---> Project tasks header files.
|  |  |--my_library.h
|  |
|  |--platfomio.ini
|
|--project2/
   |--src/
   |  |--main.c
   |
   |--platfomio.ini
```

## Copyright © 2024, Marcel Nahir Samur