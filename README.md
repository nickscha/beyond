# beyond
A C89 standard compliant, single header, nostdlib (no C Standard Library) Beyond Communication Layer (BEYOND).

<p align="center">
<a href="https://github.com/nickscha/beyond"><img src="assets/beyond.png"></a>
</p>

For more information please look at the "beyond.h" file or take a look at the "examples" or "tests" folder.

> [!WARNING]
> THIS PROJECT IS A WORK IN PROGRESS! ANYTHING CAN CHANGE AT ANY MOMENT WITHOUT ANY NOTICE! USE THIS PROJECT AT YOUR OWN RISK!

<p align="center">
  <a href="https://github.com/nickscha/beyond/releases">
    <img src="https://img.shields.io/github/v/release/nickscha/beyond?style=flat-square&color=blue" alt="Latest Release">
  </a>
  <a href="https://github.com/nickscha/beyond/releases">
    <img src="https://img.shields.io/github/downloads/nickscha/beyond/total?style=flat-square&color=brightgreen" alt="Downloads">
  </a>
  <a href="https://opensource.org/licenses/MIT">
    <img src="https://img.shields.io/badge/License-MIT-yellow.svg?style=flat-square" alt="License">
  </a>
  <img src="https://img.shields.io/badge/Standard-C89-orange?style=flat-square" alt="C Standard">
  <img src="https://img.shields.io/badge/nolib-nostdlib-lightgrey?style=flat-square" alt="nostdlib">
</p>

## **Features**
- **C89 compliant** — portable and legacy-friendly  
- **Single-header API** — just include `beyond.h`  
- **nostdlib** — no dependency on the C Standard Library  
- **Minimal binary size** — optimized for small executables  
- **Cross-platform** — Windows, Linux, MacOs 
- **Strict compilation** — built with aggressive warnings & safety checks  

## Quick Start

Download or clone beyond.h and include it in your project.

```C
#include "beyond.h" /* Beyond Communication Layer */

/* (1) Define a struct or memory block holding the data to be transmitted */
typedef struct satellite_packet
{
    long tick;
    float x;
    float y;

} satellite_packet;

/* (2) Define a transmit and/or recieve function */
int satellite_transmit(
  beyond_handle handle,
  beyond_context context, 
  void *payload, 
  int payload_capacity, 
  unsigned int *payload_size
)
{
    satellite_packet *packet = (satellite_packet *)context;

    /* It is important and required by the user to lock/unlock critical sections/mutexes */
    beyond_lock(handle);
    packet->y += 0.1f;
    beyond_unlock(handle);

    /* The size of the payload */
    *payload_size = sizeof(satellite_packet);

    beyond_memcpy(((char *)payload), packet, sizeof(satellite_packet));

    return 1;
}

int main() {
    satellite_packet context = {0};

    /* Deploy the satellite sending data to the station */
    /* This call is non-blocking!                       */
    beyond_handle satellite = beyond_satellite_deploy(
        BEYOND_COMMUNICATION_TYPE_UDP,
        satellite_transmit,
        0, /* No receive function in this example */
        beyond_address_init("127.0.0.1", 8080),
        &context);
    
    if (!satellite)
    {
        return 1;
    }

    /* Satellite might be started already        */
    /* Lock data when modifying the context data */
    beyond_lock(satellite);
    context.x = 600.0f;
    context.y = 10.0f;
    beyond_unlock(satellite);

    while (1)
    {
        beyond_lock(satellite);
        context.x += 1.0f;
        beyond_unlock(satellite);
    }

    beyond_satellite_destroy(satellite);

    return 0;
}
```

## Run Example: nostdlib, freestsanding

In this repo you will find the "examples/beyond_win32_nostdlib.c" with the corresponding "build.bat" file which
creates an executable only linked to "kernel32" and is not using the C standard library and executes the program afterwards.

## "nostdlib" Motivation & Purpose

nostdlib is a lightweight, minimalistic approach to C development that removes dependencies on the standard library. The motivation behind this project is to provide developers with greater control over their code by eliminating unnecessary overhead, reducing binary size, and enabling deployment in resource-constrained environments.

Many modern development environments rely heavily on the standard library, which, while convenient, introduces unnecessary bloat, security risks, and unpredictable dependencies. nostdlib aims to give developers fine-grained control over memory management, execution flow, and system calls by working directly with the underlying platform.

### Benefits

#### Minimal overhead
By removing the standard library, nostdlib significantly reduces runtime overhead, allowing for faster execution and smaller binary sizes.

#### Increased security
Standard libraries often include unnecessary functions that increase the attack surface of an application. nostdlib mitigates security risks by removing unused and potentially vulnerable components.

#### Reduced binary size
Without linking to the standard library, binaries are smaller, making them ideal for embedded systems, bootloaders, and operating systems where storage is limited.

#### Enhanced performance
Direct control over system calls and memory management leads to performance gains by eliminating abstraction layers imposed by standard libraries.

#### Better portability
By relying only on fundamental system interfaces, nostdlib allows for easier porting across different platforms without worrying about standard library availability.
