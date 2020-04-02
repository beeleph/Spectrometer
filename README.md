# Spectrometer
using CAEN N6740C digitizer. 

You need dll's in .exe folder to actually run the program.
You need cfg files in da build folder.

При разработке интерфейса ориентироваться на CheckKeyboardCommands в wavedump.c

Todo:
Convert wavedump, wdconfig to C++ classes QObject
1. Delete WDconfig later on
2. Delete all da garbage.
3. Fix da warnings.
4. keep roollin'
5. перенести мэин из wd.c в main.cpp. там же убрать все принтфы
6. переписать чтение конфига (parseconfigfile в wavedump.c)

Непонятки:
1. Зачем нужен allocateevent и malloc на приборе?

S
