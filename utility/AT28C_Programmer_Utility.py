import os
import sys
import time
import signal
from enum import Enum
from serial import Serial


class CommandLine:
    class Option:
        def __init__(self, option_string: str, has_argument: bool = False) -> None:
            self.option_string = option_string
            self.has_argument = has_argument
    class UnableToParseCommandError(Exception):
        __ERROR_MESSAGE = "Error: Unable to parse command!"
        def __init__(self) -> None:
            super().__init__(self.__class__.__ERROR_MESSAGE)
    def __init__(self, command_line_string: str, valid_options: list[Option]) -> None:
        self.command = ""
        self.command_args: list[str] = []
        self.options_args: dict[str, str] = {}
        self.__parse(command_line_string, valid_options)
    def __parse(self, command_line_string: str, valid_options: list[Option]) -> None:
        args = command_line_string.split(" ")
        i = 0
        while (i < len(args)):
            try:
                arg = args[i]
                if arg.startswith("-"):
                    option = None
                    if (len(arg) > 1):
                        for valid_option in valid_options:
                            if (valid_option.option_string == arg[1:]):
                                option = valid_option
                                break
                    if option is not None:
                        next_arg = ""
                        if option.has_argument:
                            if ((i + 1) >= len(args)):
                                continue
                            next_arg = args[i + 1]
                            if next_arg.startswith("-"):
                                continue
                            i += 1
                        self.options_args.update({ option.option_string: next_arg })
                else:
                    if (self.command == ""):
                        self.command = arg
                    else:
                        self.command_args.append(arg)
            finally:
                i += 1

class EepromProgrammer:
    class UnableToConnectError(Exception):
        pass
    class FileDoesNotExistError(Exception):
        pass
    class UnableToCreateFileError(Exception):
        pass
    class Mode(str, Enum):
        AT28C64 = "AT28C64"
        AT28C256 = "AT28C256"
    __COMMAND__M = "m"
    __COMMAND__R = "r"
    __COMMAND__W = "w"
    __COMMAND_RESULT__OK = "OK"
    __STATUS_MESSAGE__CLEARING = "Clearing..."
    __STATUS_MESSAGE__READING = "Reading..."
    __STATUS_MESSAGE__WRITING = "Writing..."
    __AT28C64_SIZE_IN_BYTES = 8192
    __AT28C256_SIZE_IN_BYTES = 32768
    def __init__(self, serial_port_name: str, mode: Mode = Mode.AT28C64, validate_writings: bool = True) -> None:
        self.serial_port = None
        self.mode = mode
        self.validate_writings = validate_writings
        self.__aborting = False
        try:
            self.serial_port = Serial(serial_port_name, 115200)
            self.serial_port.timeout = 1.0
            time.sleep(1.5)
            self.__discard_data()    # Discard the help message on start
            if not self.__set_mode(mode):
                raise Exception()
        except:
            raise self.__class__.UnableToConnectError()
    def __del__(self) -> None:
        if self.serial_port is not None:
            self.serial_port.close()
    def __discard_data(self) -> None:
        self.serial_port.write("\n".encode())
        time.sleep(0.25)
        while (self.serial_port.in_waiting > 0):
            self.serial_port.read_until()
    def __set_mode(self, mode: Mode) -> bool:
        self.serial_port.write(f"{self.__class__.__COMMAND__M} {mode.value}\n".encode())
        result = self.serial_port.read_until().decode().strip()
        if (result != self.__class__.__COMMAND_RESULT__OK):
            return False
        return True
    def __get_memory_size(self) -> int:
        match self.mode:
            case self.__class__.Mode.AT28C64:
                return self.__class__.__AT28C64_SIZE_IN_BYTES
            case self.__class__.Mode.AT28C256:
                return self.__class__.__AT28C256_SIZE_IN_BYTES
        return 0
    def clear(self) -> bool:
        eeprom_size = self.__get_memory_size()
        for i in range(eeprom_size):
            if self.__aborting:
                self.__aborting = False
                return False
            print(f"{self.__class__.__STATUS_MESSAGE__CLEARING} ({i + 1}/{eeprom_size}) [{(i + 1) / eeprom_size * 100.0:.0f}%]")
            if not self.write(i, 0):
                return False
        return True
    def read(self, address: int) -> (int | None):
        self.serial_port.write(f"{self.__class__.__COMMAND__R} {address}\n".encode())
        result = self.serial_port.read_until().decode().strip()
        if not result.isnumeric():
            return None
        return int(result)
    def read_file(self, file_path: str) -> bool:
        file = None
        try:
            file = open(file_path, "wb")
        except:
            raise self.__class__.UnableToCreateFileError()
        try:
            eeprom_size = self.__get_memory_size()
            for i in range(eeprom_size):
                if self.__aborting:
                    self.__aborting = False
                    return False
                print(f"{self.__class__.__STATUS_MESSAGE__READING} ({i + 1}/{eeprom_size}) [{(i + 1) / eeprom_size * 100.0:.0f}%]")
                byte = int(self.read(i))
                if byte is None:
                    return False
                file.write(byte.to_bytes())
            return True
        finally:
            file.close()
    def write(self, address: int, data: int) -> bool:
        self.serial_port.write(f"{self.__class__.__COMMAND__W} {address} {data}\n".encode())
        result = self.serial_port.read_until().decode().strip()
        if (result != self.__class__.__COMMAND_RESULT__OK):
            return False
        if (self.validate_writings and (self.read(address) != data)):
            return False
        return True
    def write_file(self, file_path: str) -> bool:
        file = None
        try:
            file = open(file_path, "rb")
        except:
            raise self.__class__.FileDoesNotExistError()
        try:
            file_bytes = file.read()
            for i in range(len(file_bytes)):
                if self.__aborting:
                    self.__aborting = False
                    return False
                file_byte = file_bytes[i]
                print(f"{self.__class__.__STATUS_MESSAGE__WRITING} ({i + 1}/{len(file_bytes)}) [{(i + 1) / len(file_bytes) * 100.0:.0f}%]")
                if not self.write(i, file_byte):
                    return False
            return True
        finally:
            file.close()
    def abort(self) -> None:
        self.__aborting = True


class Error(Enum):
    MISSING_P_OPTION = "Error: Missing '-p' option!"
    MISSING_COMMAND = "Error: Missing command!"
    INVALID_COMMAND = "Error: Invalid command!"
    MISSING_COMMAND_ARGUMENTS = "Error: Missing command argument(s)!"
    INVALID_COMMAND_ARGUMENTS = "Error: Invalid command argument(s)!"
    INVALID_PROGRAMMER_MODE = "Error: Invalid programmer mode!"
    UNABLE_TO_CONNECT_TO_PROGRAMMER = "Error: Unable to connect to programmer!"
    COMMAND_HAS_BEEN_FAILED = "Error: Command has been failed!"
    FILE_DOES_NOT_EXIST = "Error: File does not exist!"
    UNABLE_TO_CREATE_FILE = "Error: Unable to create file!"

class Command(Enum):
    CLEAR = "clear"
    READ = "read"
    READ_FILE = "read_file"
    WRITE = "write"
    WRITE_FILE = "write_file"

class Option(Enum):
    P = CommandLine.Option("p", True)
    M = CommandLine.Option("m", True)
    F = CommandLine.Option("f", False)


PROGRAM_INFO = "AT28C Programmer Utility | v1.0"


def print_help() -> None:
    print("-------------------------------")
    print(PROGRAM_INFO)
    print("-------------------------------")
    print("")
    print(f"Uasge: {os.path.basename(__file__)} -{Option.P.value.option_string} <port> [-{Option.M.value.option_string} <mode>] [-{Option.F.value.option_string}] {{command}}")
    print("")
    print("Options:")
    print(f"    -{Option.P.value.option_string} <port>                      - Specify port name to connect to.")
    print(f"    -{Option.M.value.option_string} <mode>                      - Specify mode of operation. ('mode' can be \"AT28C64\" or \"AT28C256\")")
    print("                                     [Default is: \"AT28C64\"]")
    print(f"    -{Option.F.value.option_string}                             - Disable write validation for fast writing.")
    print("")
    print("Commands:")
    print(f"    {Command.CLEAR.value}                         - Clear the EEPROM by writing zeros to every memory address.")
    print(f"    {Command.READ.value} <address>                - Read data from address 'address'.")
    print(f"    {Command.READ_FILE.value} <file_path>         - Read the EEPROM content into a file.")
    print(f"    {Command.WRITE.value} <address> <data>        - Write data 'data' at address 'address'.")
    print(f"    {Command.WRITE_FILE.value} <file_path>        - Write file's content into the EEPROM.")
    print("")

def print_error_and_terminate(error: Error, print_help_message: bool = False) -> None:
    if print_help_message:
        print_help()
    print("")
    print(error.value)
    exit(-1)



###########################################################################

if (len(sys.argv) <= 1):
    print_help()
    exit()

command_line = CommandLine(" ".join(sys.argv[1:]), [item.value for item in Option])
if Option.P.value.option_string not in command_line.options_args:
    print_error_and_terminate(Error.MISSING_P_OPTION, True)
if ((Option.M.value.option_string in command_line.options_args) and
        (command_line.options_args[Option.M.value.option_string] not in EepromProgrammer.Mode)):
    print_error_and_terminate(Error.INVALID_PROGRAMMER_MODE, True)
if (command_line.command == ""):
    print_error_and_terminate(Error.MISSING_COMMAND, True)
if command_line.command not in Command:
    print_error_and_terminate(Error.INVALID_COMMAND, True)
if (((command_line.command == Command.READ.value) and (len(command_line.command_args) < 1)) or
        ((command_line.command == Command.READ_FILE.value) and (len(command_line.command_args) < 1)) or
        ((command_line.command == Command.WRITE.value) and (len(command_line.command_args) < 2)) or
        ((command_line.command == Command.WRITE_FILE.value) and (len(command_line.command_args) < 1))):
    print_error_and_terminate(Error.MISSING_COMMAND_ARGUMENTS, True)
if (((command_line.command == Command.READ.value) and (not command_line.command_args[0].isnumeric())) or
        ((command_line.command == Command.WRITE.value) and ((not command_line.command_args[0].isnumeric()) or (not command_line.command_args[1].isnumeric())))):
    print_error_and_terminate(Error.INVALID_COMMAND_ARGUMENTS, True)

eeprom_programmer = None
try:
    serial_port_name = command_line.options_args[Option.P.value.option_string]
    validate_writings = (Option.F.value.option_string not in command_line.options_args)
    if (Option.M.value.option_string in command_line.options_args):
        mode = EepromProgrammer.Mode(command_line.options_args[Option.M.value.option_string])
        eeprom_programmer = EepromProgrammer(serial_port_name, mode, validate_writings)
    else:
        eeprom_programmer = EepromProgrammer(serial_port_name, validate_writings=validate_writings)
except EepromProgrammer.UnableToConnectError:
    print_error_and_terminate(Error.UNABLE_TO_CONNECT_TO_PROGRAMMER)
signal.signal(signal.SIGINT, (lambda signum, frame: (eeprom_programmer.abort())))

command_succeeded = False
result = None
if (command_line.command == Command.CLEAR.value):
    command_succeeded = eeprom_programmer.clear()
elif (command_line.command == Command.READ.value):
    result = eeprom_programmer.read(int(command_line.command_args[0]))
    command_succeeded = (result is not None)
elif (command_line.command == Command.READ_FILE.value):
    try:
        command_succeeded = eeprom_programmer.read_file(command_line.command_args[0])
    except EepromProgrammer.UnableToCreateFileError:
        print_error_and_terminate(Error.UNABLE_TO_CREATE_FILE)
elif (command_line.command == Command.WRITE.value):
    command_succeeded = eeprom_programmer.write(int(command_line.command_args[0]), int(command_line.command_args[1]))
elif (command_line.command == Command.WRITE_FILE.value):
    try:
        command_succeeded = eeprom_programmer.write_file(command_line.command_args[0])
    except EepromProgrammer.FileDoesNotExistError:
        print_error_and_terminate(Error.FILE_DOES_NOT_EXIST)
if not command_succeeded:
    print_error_and_terminate(Error.COMMAND_HAS_BEEN_FAILED)
if result is not None:
    print(result)
