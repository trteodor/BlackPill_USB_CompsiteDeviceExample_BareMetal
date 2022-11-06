# USB_OTG_DEVICE BARE METAL OWN STACK EXAMPLE 

**_DIRECTORY: "applications" contain provided examples_**

Directory: Driver/USBd_otg_fs/* contain prepared files for usb_otg_fs device driver
Dicrectory: applications/*/* the prepared examples have been placed...

This project was created only for educational purposes.
Created software is prepared for Board- BlackPill with STM32F401CCU6:


![BlackPill](https://github.com/trteodor/BlackPill_USB_CompsiteDeviceExample_BareMetal/blob/master/pictures/BlackPill_stm32f401ccu6.png)


**_The main example presents composite device: VCP+HID (Mouse)_**

* You are ready to start adventure with this STACK when you have this tools in your environment (environment variables)!

    * arm-none-eabi-gcc 9.3.1
    * OpenOCD 0.11.0
    * make 4.2.1

To clone this repo please use following command: 
* git clone --recurse-submodules https://github.com/trteodor/BlackPill_USB_CompsiteDeviceExample_BareMetal

Finally:

 **_Call "make" in this project at him location to show this information:_**

    Call: make help    -show this information
    To compile and the main example (VCP+HID) use following command:
    --    make CompileAppDebug App=USB_otg_Cdc_Hid flash
    --OR: make App flash              --- (DefaultApp)
    ----------------------------------------------------------------------------------------------
    How Use this build system:
    2. Call: make debugC                       -To remove build folder, compile Default App with Debug information and flash Firmware in MCU
    7. Call: make CompileApp App=AppName       -To compile Choosen App - Firmware for MCU (AppName its Parameter) (without debug information)
    9. Call: make CompileAppDebug App=AppName  -To compile Choosen App with Debug information and flash Firmware in MCU (AppName its Parameter)
    ------------------------------------
    15. Call: make cleanR         -Remove build Release Folder
    15. Call: make cleanD         -Remove build Debug Folder


This project use my own build system created using makefiles if you want you can adapt him to your projects





I Also prepare template for unit tests - but i didn't write any test. Im'too lazy... as you know writing tests taking a bit time..

**_If you have question please open issue to this repository._**



# General References:

* https://wydawnictwo.btc.pl/elektronika/204656-usb-dla-niewtajemniczonych-w-przykladach-na-mikrokontrolery-stm32-e-book.html

* https://www.usb.org/

* https://github.com/hathach/tinyusb?fbclid=IwAR2eV_u5AAk7GbArpe6f22GNZaFtwJsuoiuO3vi2VzL7FNbEQ0NLS_DfuY4

* https://www.elektroda.pl/rtvforum/topic3310418.html

* https://github.com/x893/STM32F103-DualCDC

* https://sudonull.com/post/68144-CDC-MSC-USB-Composite-Device-on-STM32-HAL

**How to tinyUSB in STM32CubeIDE:**

* https://anvilelectronics.ovh/tinyusb-na-stm32-device-cdc/


**PyUsb:**

* https://github.com/pyusb/pyusb/blob/master/docs/tutorial.rst

* https://developer.cisco.com/docs/the-cisco-pyusb-developer-guide/

* https://www.ontrak.net/LibUSBPy.htm



The last only as Curiosity: 

https://github.com/alambe94/I-CUBE-USBD-Composite?fbclid=IwAR2vQEj_NNZ7KUWyuTjpTuLJg-uFVfsuOl11_1ine6B1eP6DTmZ_XS_4VLw