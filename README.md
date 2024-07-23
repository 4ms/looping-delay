## Looping Delay Firmware ##

This is the firmware for the Looping Delay eurorack module from 4ms Company.

The same firmware runs on the pre-built module as well as the DIY Kit.

Note that there are two variations of the kit: v1.0 and v1.1. Make sure you
know what version you have.

### Cloning the repo

To clone the repo, don't forget to clone the submodules:

```
git clone https://github.com/4ms/looping-delay
cd looping-delay
git submodule update --init
```

### Building

To build:

```
cmake -B build
cmake --build build
```

There is also a Makefile if you prefer that syntax (it just wraps the cmake calls):

```
make
```

### Selecting the Brainboard version

There are three possible brainboards to build for:
 - Brainboard v1.0 (F723 chip)
 - Brainboard v1.1-rc4 (F746 chip)
 - Brainboard MP15x (MP153 chip)

Building using the above commands will create firmware for the Brainboard v1.0
and v1.1-rc4. By default the MP15x board is not enabled because this is
aspecially-available upgraded Brainboard, running a Cortex-A7 chip.

If you want to just build for your board (which will speed up the build), you
can select it like this:

Just build Brainboard v1.0:
```
cmake --fresh -B build -DBRAINBOARD1_0=ON -DBRAINBOARD1_1=OFF
```

Just build Brainboard v1.1:
```
cmake --fresh -B build -DBRAINBOARD1_0=OFF -DBRAINBOARD1_1=ON
```

Just build Brainboard MP15x:
```
cmake --fresh -B build -DBRAINBOARD1_0=OFF -DBRAINBOARD1_1=OFF -DBRAINBOARD_MP15x=ON
```


The .elf files are found here:
 - Brainboard v1.0: `build/f723/723.elf` 
 - Brainboard v1.1: `build/f746/746.elf`
 - Brainboard MP15x: `build/mp153/mp153.elf`

The elf file can be flashed using normal means to your Brainboard via the SWD header.

### Flashing with USB-DFU

You can build a .bin file containing the bootloader and the application, and
flash it using the built-in USB DFU bootloader. To do this:

For Brainboard v1.0:

```
cmake --build build --target 723-combo
```

For Brainboard v1.1:

```
cmake --build build --target 746-combo
```


or for all targets:

```
make combo
```

This generates .bin files here:
 - Brainboard v1.0: `build/f723/723-combo.bin` 
 - Brainboard v1.1: `build/f746/746-combo.bin`

You can then use a DFU tool to flash this to the device. 
First, reset the device into DFU mode by holding down the RESET button, then
pressing and releasing the USB BOOT button. Finally, release the RESET button.
(Note: the names of these buttons were written incorrectly on the PCB, so if
you have used DFU bootloaders and think the previous sentence is a typo, don't
second guess it!).

Now you can load the combo .bin file to the device at address 0x08000000. An
easy way is using a web dfu bootloader such as
https://devanlai.github.io/webdfu/dfu-util/


### Building a wav file for audio bootloader

For Brainboard v1.0:

```
cmake --build build --target 723.wav
```

For Brainboard v1.1:

```
cmake --build build --target 746.wav
```

or for all targets:

```
make wav
```


### Some notes on the ISR handler

The audio callback is called by DMA interrupts (DMA2 Stream1 ISRs for Transfer Complete, and for Half-Transfer Complete).
We use lambdas and std::function to store this callback, as you can see in `src/audio_stream.hh`. This makes for some
cleaner code, as we can pass references to all the stuff the callback needs, and avoid any global data (see main.cc).

While this might seem like use std::function introduces overhead in the most critical section
of code, inspecting the generated assembly reveals that the compiler does an
excellent job of optimizing. For the F723 target, here is the assembly:


```asm
; SAI DMA ISR: [IRQ57]
; vector table entry for IRQ 57 (DMA2_Stream1_IRQn):
 0800C124   DC32           0x0800E935

; When the IRQ is asserted, the processor jumps to our ISR Handler:
 0800E934   LDR            R0, =0x200009A0              ; Load the address of our function object 
 0800E936   LDR            R3, [R0, #-8]
 0800E93A   BX             R3							; jumps to the function object at 0x0800D87C
 0800D87C   PUSH           {R3-R5, LR}                  ; save the stack (ISRs normally have to do this)
 080020CE   LDR            R3, [R0]                     ; load our function object to R3 (this contains the state and context we use in the ISR)
 080020D0   MOV            R4, R0                       ; stash the function object address for later use

; actual ISR starts here (checks DMA flags, jump to our audio routine)
; then ends:

  08002146   POP            {R3-R5, PC}                 ; restore the stack and return to normal execution
 ```

 So, you can see there are about 5 instructions of overhead (the stack setup/teardown would be a part of any ISR). 
 One of them is a jump, which makes the I-cache work a little harder, but the other 4 are used to access the data
 we need to run our ISR (i.e. the audio buffers we need to read/write, our parameters, which DMA flags to check, etc).
 Any non-trivial ISR would also need to access some sort of data, which would presumably take about the same amount
 of processing to perform (e.g. accessing some global variables).


