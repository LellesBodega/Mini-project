DTEK-V Embedded Image Processor (Kernel Convolution)

This firmware allows the user to perform 2D image convolution using various kernels (filters) directly on the DTEK-V processor. The user controls the operation using the slide switches (SW) and the push button (BTN) on the DE10-Lite board.

The system operates in a single, continuous loop that reads the state of the switches and button, updates the menu state, and executes a selected action (image processing or reset) upon a button press. It can also perform a back-to-back input performing two kernels.

The image is pre-loaded. The switches are used to perform operations. The image before and after remains in raw format. 
- SW[1:0]: 00=Edge, 01=Box, 10=Gauss, 11=Sharp.
- SW[2]: Kernel Size, 0=3x3, 1=5x5.
- SW[3]: Set to 1 to enable 'Process Image' action.
- SW[4]: Set to 1 to enable back-to-back, aka chain.
- SW[6]: Set to 1 to enable 'Reset Image' action.

The operation will only be performed when BTN[1] is pressed.

Step-by-Step Guide
1. Power On: Power on the DE10-Lite board and view the terminal output.
   (Note the Address: The program will print the memory address for the output_img buffer. 
   Copy this address, as you will need it to download your result).

   --> Use this address with dtekv-download:
   output_img address: 0x........

2. Select a Filter: Use SW[1:0] and SW[2] to choose the desired filter and size. 
   The LEDs will light up to confirm your selection.

3. Process the Image:
   - Set SW[3] to 1.
   - Press the push button BTN[1].

   The terminal will confirm that processing is complete.

4. Perform back-to-back:
   - Set SW[4] to 1. 
   - Press the push button BTN[1].
   - You should then select another kernel to apply. 

5. Download the Result:
   - On your host PC, open a terminal.
   - Run the dtekv-download command using the address from Step 2.
   - Example Command: dtekv-download my_output_image.raw <address_from_startup> 65536