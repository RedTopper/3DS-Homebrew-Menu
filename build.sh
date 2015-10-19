source ~/.bashrc
make clean
osascript -e 'tell application "System Events" to keystroke "k" using command down'
make -s
