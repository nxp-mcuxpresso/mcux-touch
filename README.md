# NXP Middleware NXP Touch Library 
The NXP Touch software library is designed to speed development of your touch applications and is ideal for use with NXP MCUs. Available as a source code, this software download features touch detection algorithms.
The NXP Touch software library employs a modular architecture with a variety of touch centric controls, modules, and electrode data objects, enabling integrated and customizable features.
Touch software is based on a layered architecture with data types resembling an object-oriented approach and uses plain C language to configure electrodes, modules and 
controls. The library code is well suited for use in RTOS-based multi-tasking applications and in C++ object-oriented applications.
The touch sensing algorithms contained in the library utilize either dedicated Touch Sensing Interface (TSI) module available on many of the NXP controllers or GPIO signals to detect finger touch, movement or gestures.

## Documentation
The NXP Touch Library Reference Manual is located in touch middleware, to open it go to:
..\freemaster\html\index.html

## License
This repository is under **LA_OPT_Online Code Hosting NXP_Software_License**.

## Examples
The examples are placed on the repository [mcux-sdk-examples](https://github.com/nxp-mcuxpresso/mcux-sdk-examples).
The example list with description:

- **touch_sensing**
  - Example demonstrates the Touch library features enabling to control the RGB LED using touch buttons, slider or rotary elements designed ideally for the FRDM-TOUCH plug-in board together with FRDM-KE15Z, FRDM-KE16Z, FRDM-KE17Z or FRDM-KE17Z512 baseboards.
  
- **touch-haptic**
  - This demo aims to provide feedback to the user's interaction by vibration or sound. This demo is intended for the FRDM-KE15Z board combined with the FRDM-HAPTIC and FRDM-TOUCH plug-in boards.