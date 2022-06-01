""" Client.py

General information
----------
Author:
    Tomuta Gabriel
Date:
    01.06.2022
Version: 1.0

Description
----------
Script contains class that creates the window and
handles all the communication.
To create a client, check the outside method:
    test_client()
"""

from enum import Enum
from threading import Thread
import socket
import time

from PyQt5 import uic, QtWidgets
from PyQt5.QtWidgets import *
from PyQt5.QtCore import *

SERVER_IP = 'localhost'
SERVER_PORT = 12345

NAME_SIZE = 10
BUFF_LEN = 512


class Status_Codes(Enum):
    ConnAccepted = 0  # Client connected successfully
    Disconnect = 1  # Client disconnected from the server
    InvalidName = 2  # Received name is taken
    MissingName = 3  # Name not received, stop connection attempt

    MsgReceived = 4  # Message successfully received
    MsgSuccess = 5  # Message successfully arrived at the destination
    MsgFailed = 6  # Message could not be sent
    InvalidDest = 7  # A client with the specified name does not exist

    ReqClients = 8  # A client requested a list with all the connected clients


class ClientWindow(QWidget):
    def __init__(self):
        super(ClientWindow, self).__init__()
        uic.loadUi(r"ClientInterface.ui", self)
        self.setWindowTitle("RC Client")

        # Widgets
        self.QtButt_Connect = self.findChild(QPushButton, "QtButtConnect")
        self.QtButt_Send = self.findChild(QPushButton, "QtButtSend")
        self.QtButt_Clients = self.findChild(QPushButton, "QtButtClients")

        self.QtCheckConnected = self.findChild(QCheckBox, 'QtCheckConnected')
        self.QtClientName = self.findChild(QLineEdit, 'QtClientName')
        self.QtDest = self.findChild(QLineEdit, 'QtDest')
        self.QtMessage = self.findChild(QLineEdit, 'QtMessage')
        self.QtChat = self.findChild(QTextEdit, 'QtChat')
        self.QtConsole = self.findChild(QTextEdit, 'QtConsole')

        # Members
        self.client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.server_ip = SERVER_IP
        self.server_port = SERVER_PORT
        self.name = None
        self.connected = False
        self.expecting_confirmation = False
        self.confirmation = False
        self.last_chat_name = ''
        self.last_chat_dest = ''

        # Signals
        self.QtButt_Connect.clicked.connect(self.on_clicked_connect)
        self.QtClientName.returnPressed.connect(self.on_clicked_connect)
        self.QtDest.returnPressed.connect(self.QtMessage.setFocus)
        self.QtButt_Send.clicked.connect(self.on_clicked_send)
        self.QtMessage.returnPressed.connect(self.on_clicked_send)
        self.QtButt_Clients.clicked.connect(self.on_clicked_clients)

        # Init
        self.init_window()

        # Main loop
        self.run_loop = True
        self.loop = Thread(target=self.main_loop)

    # Initialization ---------------------------------------------------------------------------------------------------
    def init_window(self):
        self.show()

    def main_loop(self):
        while self.run_loop:
            if self.connected:
                self.receive_data()
                time.sleep(0.1)

    # Events -----------------------------------------------------------------------------------------------------------
    def closeEvent(self, event):
        exit_result = QMessageBox.question(self, "Closing..",
                                           "Do you wish to close the client?",
                                           buttons=QMessageBox.Yes | QMessageBox.No)

        if exit_result == QMessageBox.Yes:
            self.disconnect_client()
            event.accept()
        elif exit_result == QMessageBox.No:
            event.ignore()

    # Methods ----------------------------------------------------------------------------------------------------------
    def connect_client(self) -> bool:
        """ Method used to connect to the server at the given ip and port """
        self.write_to_console(f"Attempting connection to server on {self.server_ip}:{self.server_port}.")
        try:
            self.client.connect((str(self.server_ip), int(self.server_port)))
        except socket.error:
            self.write_to_console('Could not connect to server. Check if the server is on.')
            return False

        # Send name
        if self.client.sendall(str.encode(self.name)) is not None:
            self.write_to_console("Name could not be send.")
            return False

        # Expect feedback
        time.sleep(0.1)
        data = self.client.recv(BUFF_LEN)
        data = data.decode("utf-8")

        if int(data) != Status_Codes.ConnAccepted.value:
            self.write_to_console(f"Connection failed with error code {data}")
            return False

        self.client.settimeout(0.00001)
        self.run_loop = True
        self.loop.start()
        return True

    def disconnect_client(self) -> bool:
        """ Method used to disconnect from the server """
        if self.connected:
            self.send_status(Status_Codes.Disconnect)
            self.run_loop = False
            time.sleep(0.1)
            self.client.close()
        return True

    def send_msg(self, msg) -> bool:
        """
            Method used to send a message to the server to be forwarded
            Message format: <Src>|<Dest>|<Msg>
        """
        # Prepare message
        msg = "{}|{}|{}".format(self.name, self.QtDest.text(), msg)

        # Send message
        if self.client.sendall(str.encode(msg)) is not None:
            self.write_to_console("Message could not be send")
            return False

        # Expect feedback that the message got to the server
        self.confirmation = False
        self.expecting_confirmation = True
        time.sleep(0.2)
        return self.confirmation

    def send_status(self, status: Status_Codes) -> bool:
        """
            Method used to send a code to the server.
        """
        if self.client.sendall(str.encode(str(status.value))) is not None:
            self.write_to_console("Response could not be send")
            return False
        return True

    def receive_data(self):
        """
            Method that with every run checks if data was received.
        If it was received, looks if the data is a message or a code and treats them accordingly
        """
        try:
            data = self.client.recv(BUFF_LEN)
        except socket.timeout as e:
            pass
        else:
            data = data.decode("utf-8")
            # self.write_to_console("Message received: {}".format(data))

            split_data = data.split('|')
            # Message received
            if len(split_data) == 2:
                # Format <Src>|<Msg>
                self.write_in_chat(split_data[0], self.name, split_data[1])

                # Send confirmation
                self.send_status(Status_Codes.MsgReceived)

            # Code received
            else:
                if self.expecting_confirmation:  # If message was sent recently and awaiting confirmation
                    if int(data) != Status_Codes.MsgSuccess.value:
                        self.write_to_console(f"Message failed with error code {Status_Codes(int(data)).name}")
                        self.confirmation = False
                    else:
                        self.confirmation = True
                    self.expecting_confirmation = False
                else:  # If other code was send
                    self.write_to_console(f"Received the following status code: {Status_Codes(int(data)).name}.")

    def write_to_console(self, msg):
        local_time = time.strftime("%d.%m %H:%M:%S", time.localtime())
        self.QtConsole.append("{0} - {1}".format(local_time, msg))

        # Comment this
        print("{0} - {1}".format(local_time, msg))

    def write_in_chat(self, name: str, dest: str, message):
        if self.last_chat_name != name or self.last_chat_dest != dest:
            self.last_chat_name = name
            self.last_chat_dest = dest
            self.QtChat.append('<html><b> # {0} -- {1}:</b</html>'.format(name, dest))
        self.QtChat.append('<html><i>{}</i</html>'.format(message))
        self.QtChat.verticalScrollBar().setValue(self.QtChat.verticalScrollBar().maximum())

    # Slots ------------------------------------------------------------------------------------------------------------
    @pyqtSlot()
    def on_clicked_connect(self):
        # Connect to the server
        if not self.connected:
            if self.QtClientName.text() is '':
                self.write_to_console('Enter the client name before connecting.')
                return

            if not self.QtClientName.text()[0].isalpha():
                self.write_to_console('Name does not start with an alphabetic character.')
                return

            if len(self.QtClientName.text()) > NAME_SIZE:
                self.write_to_console('Name exceeds the allowed maximum.')
                return

            self.name = self.QtClientName.text()
            self.write_to_console('Connecting to server...')

            if not self.connect_client():
                return

            self.connected = True
            self.write_to_console('Client connected successfully with the name \"{}\".'.format(self.name))
            self.QtClientName.setEnabled(False)
            self.QtButt_Clients.setEnabled(True)
            self.QtButt_Send.setEnabled(True)
            self.QtDest.setEnabled(True)
            self.QtDest.setFocus()
            self.QtMessage.setEnabled(True)
            self.QtCheckConnected.setChecked(True)
            self.QtButt_Connect.setText('Disconnect')

        # Disconnect from the server
        else:
            if not self.disconnect_client():
                self.write_to_console('There appear to be problems with the disconnection of the client..')
                return

            self.connected = False
            self.write_to_console('Client disconnected successfully.')
            self.QtClientName.setEnabled(True)
            self.QtButt_Clients.setEnabled(False)
            self.QtButt_Send.setEnabled(False)
            self.QtDest.setEnabled(False)
            self.QtDest.setText('')
            self.QtMessage.setEnabled(False)
            self.QtMessage.setText('')
            self.QtCheckConnected.setChecked(False)
            self.QtButt_Connect.setText('Connect')

    @pyqtSlot()
    def on_clicked_send(self):
        assert self.connected, 'Client not connected to the server.'
        if self.QtDest.text() is '':
            self.write_to_console('You have to enter the destination client\'s name first.')
            return

        if self.QtMessage.text() is '':
            self.write_to_console('You have to enter a message first.')
            return

        self.write_to_console('Sending the message to {}...'.format(self.QtDest.text()))

        # Successful
        if self.send_msg(self.QtMessage.text()):
            self.write_to_console('Message sent successfully!')
            self.write_in_chat(self.name, self.QtDest.text(), self.QtMessage.text())
            self.QtMessage.setText('')

        # Unsuccessful
        else:
            self.write_to_console('Error in sending the message.')

        # TODO check why sometimes it fails

    @pyqtSlot()
    def on_clicked_clients(self):
        # TODO for future
        print('Clicked clients!!')


def test_client():
    app = QApplication([])
    window = ClientWindow()
    app.exec_()


if __name__ == "__main__":
    test_client()
