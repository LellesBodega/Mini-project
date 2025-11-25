# DTEK-V Embedded Image Processor

## What the project does
The DTEK-V Embedded Image Processor is a firmware application designed for performing 2D image convolution using various kernels (filters) directly on the DTEK-V processor. It allows users to process images in real-time using a simple interface on the DE10-Lite board.

## Why the project is useful
Key features and benefits include:
- **Real-time image processing**: Users can apply different filters to images on-the-fly.
- **Multiple filter options**: Supports edge detection, box blur, Gaussian blur, and sharpening.
- **User-friendly interface**: Control the processing through slide switches and a push button.
- **Embedded system**: Designed specifically for the DTEK-V architecture, making it efficient for embedded applications.

## How users can get started
### Installation/Setup Instructions
1. **Clone the repository**:
   ```
   git clone <repository-url>
   cd Mini-projekt
   ```

2. **Build the project**:
   Use the provided Makefile to compile the project:
   ```
   make
   ```

3. **Load the image**:
   The image is pre-loaded in the firmware. You can convert your own images to the required format using the provided Python scripts in the `tools` directory.

4. **Run the application**:
   Power on the DE10-Lite board and follow the terminal output for instructions. Use the slide switches to select filters and press the button to process the image.

### Usage Examples
- **Select a Filter**: Use SW[1:0] to choose the desired filter (00=Edge, 01=Box, 10=Gauss, 11=Sharp).
- **Set Kernel Size**: Use SW[2] to select the kernel size (0=3x3, 1=5x5).
- **Process Image**: Set SW[3] to 1 and press BTN[1].
- **Reset Image**: Set SW[6] to 1 to reset the image.

## Where users can get help
For support and additional documentation, refer to:
- [DTEK-V Documentation](docs/)
- [Contribution Guidelines](CONTRIBUTING.md)

## Who maintains and contributes
This project is maintained by:
- Artur Podobas
- Wiktor Szczerek
- Pedro Antunes
- Lelle

If you would like to contribute, please refer to the [CONTRIBUTING.md](CONTRIBUTING.md) file for guidelines on how to get involved.
