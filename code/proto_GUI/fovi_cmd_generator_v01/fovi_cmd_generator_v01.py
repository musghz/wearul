
# Generate communication packet for fovi devices.
#
# Protovol version is V03. Enter the data and click "generate" button. Click 
# on the relevant "copy" button to copy the contents of that box to clipboard.
#
# MUST use image sixe 350x350 pixels and image type .gif. Designed for display
# size 1920 X 1080
#
# Pyinstall instructions
# -copy file to users folders otherwise it cannot find them
# -in Spyder, do: ! pyinstaller --debug all fovi_cmd_generator_v01.py
# -use the contents of the dist folder to distribute
# -copy any dependent image, text files etc into dist before distributing
# -Is VC++ executable needed? Maybe only for onefile option
#
# TODO: 
# -Cleanup
# -Add source destination row
# -new image
#
# Links:
# https://www.edureka.co/blog/tkinter-tutorial/
# https://www.delftstack.com/tutorial/tkinter-tutorial/tkinter-entry/
# https://www.delftstack.com/tutorial/tkinter-tutorial/tkinter-combobox/
# https://effbot.org/tkinterbook/entry.htm
# Images and adanced grid stuff
# https://www.geeksforgeeks.org/python-add-image-on-a-tkinter-button/
# https://www.c-sharpcorner.com/Blogs/basics-for-displaying-image-in-tkinter-python
# https://www.effbot.org/tkinterbook/grid.htm
# 
# Created 24 Mar 2020
#
# @author Mustafa Ghazi

import tkinter as tk
from tkinter import ttk

imgFilePath = "fovi_diagram_v01.gif"
window = tk.Tk()

window.title("fovi_cmd_generator_v01")

window.geometry('700x400')





listOfMotorCommands = ["0 off",
                        "1 constant low",
                        "2 constant medium",
                        "3 constant high",
                        "4 on/off, 1sec, low"
                        "5 on/off, 1 sec, medium",
                        "6 on/off, 1 sec, high",
                        "7",
                        "8",
                        "9",
                        "10",
                        "11",
                        "12",
                        "13",
                        "14",
                        "15"]

# labels
lblx= tk.Label(window, text="Hello")
lblAddr = tk.Label(window, text="Slave node addr")
lblM0 = tk.Label(window, text="Motor 0")
lblM1 = tk.Label(window, text="Motor 1")
lblM2 = tk.Label(window, text="Motor 2")
lblM3 = tk.Label(window, text="Motor 3")
lblCmdHi = tk.Label(window, text="Cmd checksum high")
lblCmdLo = tk.Label(window, text="Cmd checksum low")
lblValueOfCmdHi = tk.Label(window, text="0")
lblValueOfCmdLo = tk.Label(window, text="0")
lblRespHi = tk.Label(window, text="Resp checksum high")
lblRespLo = tk.Label(window, text="Resp checksum low")
lblValueOfRespHi = tk.Label(window, text="0")
lblValueOfRespLo = tk.Label(window, text="0")
lblCommand = tk.Label(window, text="Command")
lblResponse = tk.Label(window, text="Response")

#combo boxes
comboM0 = ttk.Combobox(window, values=listOfMotorCommands)
comboM1 = ttk.Combobox(window, values=listOfMotorCommands)
comboM2 = ttk.Combobox(window, values=listOfMotorCommands)
comboM3 = ttk.Combobox(window, values=listOfMotorCommands)
comboAddr = ttk.Combobox(window, values=["0 upper arm",
                                      "1 forearm"])

# entries or textboxes
commandString = tk.StringVar()
responseString = tk.StringVar()
entryCommand =tk.Entry(window, width=12, textvariable=commandString)
entryResponse =tk.Entry(window, width=12, textvariable=responseString)
commandString.set("invalid")
responseString.set("invalid")

# position labels
lblx.grid(column=0, row=0)
lblAddr.grid(column=0, row=1)
lblM0.grid(column=0, row=2)
lblM1.grid(column=0, row=3)
lblM2.grid(column=0, row=4)
lblM3.grid(column=0, row=5)
lblCmdHi.grid(column=0, row=6)
lblCmdLo.grid(column=0, row=7)
lblRespHi.grid(column=0, row=8)
lblRespLo.grid(column=0, row=9)
lblCommand.grid(column=0, row=10)
lblResponse.grid(column=0, row=11)

# position data
comboAddr.grid(column=1, row=1)
comboM0.grid(column=1, row=2)
comboM1.grid(column=1, row=3)
comboM2.grid(column=1, row=4)
comboM3.grid(column=1, row=5)
lblValueOfCmdHi.grid(column=1, row=6)
lblValueOfCmdLo.grid(column=1, row=7)
lblValueOfRespHi.grid(column=1, row=8)
lblValueOfRespLo.grid(column=1, row=9)
entryCommand.grid(column=1, row=10)
entryResponse.grid(column=1, row=11)

# set some default values
comboM0.current(0)
comboM1.current(0)
comboM2.current(0)
comboM3.current(0)
comboAddr.current(0)


def clicked():

    lblx.configure(text="Sample Entry")
    comboAddr.current(1)
    comboM0.current(4)
    comboM1.current(4)
    comboM2.current(4)
    comboM3.current(4)
    
def generate():
    # command, source = 1    
    #get() for value, current() for index
    rawSumCmd = 1 + comboAddr.current() + comboM0.current() + comboM1.current() + comboM2.current() + comboM3.current()
    hiCmd = int(rawSumCmd/16)
    loCmd = int(rawSumCmd)%16
    # source is 0, so above sum - 1
    rawSumResp = rawSumCmd - 1
    hiResp = int(rawSumResp/16)
    loResp = int(rawSumResp)%16
    print("Cmd: checksum high = " + str(hiCmd) + ", checksum low = " + str(loCmd))
    print("Resp: checksum high = " + str(hiResp) + ", checksum low = " + str(loResp))
    
    lblValueOfCmdHi.configure(text=str(hiCmd))
    lblValueOfCmdLo.configure(text=str(loCmd))
    lblValueOfRespHi.configure(text=str(hiResp))
    lblValueOfRespLo.configure(text=str(loResp))
    
    tempCommandString = "z1" + str(comboAddr.current()) + str(comboM0.current()) + str(comboM1.current()) + str(comboM2.current()) + str(comboM3.current()) + convertIntToChar(hiCmd) + convertIntToChar(loCmd) + "s"
    tempResponseString = "z0" + str(comboAddr.current()) + str(comboM0.current()) + str(comboM1.current()) + str(comboM2.current()) + str(comboM3.current()) + convertIntToChar(hiResp) + convertIntToChar(loResp) + "s"
      
    commandString.set(tempCommandString)
    responseString.set(tempResponseString)
    
def convertIntToChar(someInt):
    someChar = "?"
    chars=["0","1","2","3","4","5","6","7","8","9","A","B","C","D","E","F"]
    if(int(someInt)>=0) and (int(someInt)<=15):
        someChar = chars[int(someInt)]
        
    return someChar

def copyCommandToClipboard():
    #window.withdraw()
    window.clipboard_clear()
    window.clipboard_append(commandString.get()) 
    
def copyResponseToClipboard():
    #window.withdraw()
    window.clipboard_clear()
    window.clipboard_append(responseString.get())
    
# buttons
btn = tk.Button(window, text="Sample settings", command=clicked)
btnGenerate = tk.Button(window, text="Generate", command=generate)
btnCopyCmd = tk.Button(window, text="Copy", command=copyCommandToClipboard)
btnCopyResp = tk.Button(window, text="Copy", command=copyResponseToClipboard)

btn.grid(column=1, row=0)
btnGenerate.grid(column=1, row=12)
btnCopyCmd.grid(column=2, row=10)
btnCopyResp.grid(column=2, row=11)


#load image
#canvas = tk.Canvas(window, width=350, height=350)
#canvas.pack()
img = tk.PhotoImage(file="fovi_diagram_v01.gif")
#canvas.create_image(20,20, anchor="nw", image=img)
#canvas.create_image(20,20, image=img)
btnImage = tk.Button(window, text = 'Click Me !', image = img)
btnImage.grid(column=3, row=0, rowspan=20)


window.mainloop()

# Header: ‘z’
#Source: 1 if the message is from the master node, 0 if the message is from a slave node.
# Slave node address: 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, A, B, C, D, E, or F
# Command motor 0: 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, A, B, C, D, E, or F
# Command motor 1: 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, A, B, C, D, E, or F
# Command motor 2: 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, A, B, C, D, E, or F
# Command motor 3: 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, A, B, C, D, E, or F
# Checksum high: Convert all characters to decimal first. Then calculate (source + address + cmd1 + cmd2 + cmd3 + cmd4) / 16. Convert “checksum high” to decimal to compare.
# Checksum low: Convert all characters to decimal first. Then calculate (source + address + cmd1 + cmd2 + cmd3 + cmd4) % 16. Convert “checksum low” to decimal to compare.
# Terminator: ‘s’
