from PyQt5.QtWidgets import QApplication
from Client import ClientWindow
app = QApplication([])


def run_multiple_clients():
    """ Method creates multiple client windows """
    num_of_clients = 2
    windows = []

    for cl in range(num_of_clients):
        windows.append(ClientWindow())

    app.exec_()


if __name__ == '__main__':
    run_multiple_clients()
