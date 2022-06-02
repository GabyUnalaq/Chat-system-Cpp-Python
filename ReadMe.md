# Chat system using C++ and Python

This project is a simple chat system build for Windows that has it's server written in C++ and it's Client in Python. It allows for multiple clients to be connected at the same time and it also allows them to message each other privately.

## Getting started
### Clone the repository
A first step towards building the project it's to download the repository. You can do this by simply clicking "Code" and downloading it or oppening via Github Desktop.

Another way is to clone it using the github command line:
```bash
git clone https://github.com/GabyUnalaq/Chat-system-Cpp-Python.git
```

### Building the Server
The Server was generated using CMake gui version 3.17.0 and it was built using Visual Studio 2019.
After running CMake in the server folder and building the solution, the server is ready to be used.

### Building the Client
As for the Client, the window was designed using the QT framework with Python version 3.7.7. To get the required libraries, run the following command in your Command Prompt:
```bash
pip install pyqt5 threaded sockets
```
After that, you can:
 - run `Client.py` for a single Client
 - run `main.py` for multiple Clients on the same device
 - build yourself an executable using the `ExeMaker_pyinstaller.bat`. This will create a folder called `dist` and inside you will find the final application. To run, the `ClientInterface.ui` has to be in the same folder as the compiler executable.

## Contributing
If you have a suggestion that would make this better, please fork the repo and create a pull request. You can also simply open an issue and we will continue from there. 

Don't forget to give the project a star! Thanks again!

## Contact
Tomuta Gabriel - [Linkedin](https://www.linkedin.com/in/gabyunalaq/) - [RovisLab](https://www.rovislab.com/tomuta_gabriel.html)

Project Link: [https://github.com/GabyUnalaq/Chat-system-Cpp-Python](https://github.com/GabyUnalaq/Chat-system-Cpp-Python)

## License
Distributed under the [MIT](https://choosealicense.com/licenses/mit/) License. See `LICENSE` for more information.
