#! /usr/bin/env python3
import r2pipe
import os
import argparse
import subprocess
from time import sleep

SLEEP_TIME = 1.5

pwd = os.getcwd()

in_file = False
out_file = False
hex_file = False
hex_data = False
address = False
target_binary = False

r2 = False


# TODO:
#	Function to actually push data to r2, and into the line requested
#	Function to make sure what line we want to push data to
#	Function to make display line(address) that we want to push to
#	Function to double check if that line is empty or not
#	Function to compile if the user wishes
#	Function to get hex data if the user wishes
#

def clear_screen():
	if os.name == "nt":
		os.system("cls")
	else:
		os.system("clear")
	return

def confirm(message):
	clear_screen()
	print(message)
	user_input = input(">")
	if (user_input.lower() == "y"):
		return True
	elif (user_input.lower() == "n"):
		return False
	else:
		print("Invalid input! Please enter Y or N")
		return confirm(message)

def pause():
	input("Press any button to continue...")
	return

def compile_file():
	global in_file
	global out_file
	clear_screen()
	if (in_file):
		os.system(f"gcc -c -fno-stack-protector -nostdlib {in_file} -o ./out.out")
		out_file = "./out.out"
		if (os.path.exists(out_file)):
			print("Finished!")
			pause()
		else:
			print("Did not compile!")
			print("Unsure why, you can attempt to do it your self with the follow line:\n gcc -c -fno-stack-protector -nostdlib (src) -o (out)")
			pause()
		return
	else:
		print("You don't have a file selected!")
		pause()
		return

def get_hex_data():
	global hex_data
	global hex_file
	clear_screen()
	if (hex_data):
		if (confirm("You have hex data already! Do you want to overwrite it?(Y/N)")):
			# Continue
			pass
		else:
			print("Not overwriting existing hex data!")
			sleep(SLEEP_TIME)
			return
	else:
		if (out_file):
			os.system(f"objcopy -O binary --only-section=.text {out_file} ./out.bin")
			clear_screen()
			hex_file = "./out.bin"
			if (os.path.exists(hex_file)):
				print("Finished!")
				pause()
			else:
				print("Did not work! You can attempt with the following line:\nobjcopy -O binary --only-section=.text (src) (out)")
				pause()	
			return
		else:
			print("You do not have a file available or selected for this operation!")
			pause()
			return

def read_hex_data():
	global hex_file
	global hex_data
	clear_screen()
	if (hex_file == False):
		print(os.system("ls"))
		print("You do not have a file available or selected for this operation!")
		pause()
		return
	print("Reading hex data!")

	hex_data = subprocess.run(f"xxd -p {hex_file}", shell=True, capture_output=True)
	hex_data = hex_data.stdout.decode("utf-8")

	print(f"Hex File: {hex_file}")
	print(f"Directory: {os.system('ls')}")

	os.system(f"hexdump -C {hex_file}")

	print()
#	 print(hex_data)
	pause()
	return

def confirm_address(search=False):
	global address
	clear_screen()
	print("Please enter and address or type 'search' to browse for an address!")
	print("Note to quit, please ctrl+c")
	try:
		address = input(">")
		if (address.lower() == "search"):
			print("Opening r2 in a new Terminal window to search for an address!")
			print("Once you quit r2, you will come back to this menu, so just find the address you want and copy it to your clipboard, then paste it here!")
			pause()
			# Launch r2 in a new Terminal window using AppleScript
			script = f'''tell application "Terminal"
						activate
						do script "r2 -A {target_binary}"
						end tell'''
			subprocess.run(["osascript", "-e", script])
			pause()

		if (not confirm(f"You entered:\n{address}\nIs this what you meant?(Y/N)")):
			address=False
			check_address()
		else:
			return
	except KeyboardInterrupt:
		print("\nExiting!")
		sleep(SLEEP_TIME)
		main()

def check_address():
	global address
	clear_screen()
	if (address):
		r2 = r2pipe.open(f"{target_binary}")
		print(r2.cmd(f"s {address}"))
		print(r2.cmd("px"))
		r2.quit()
		pause()
		return
	else:
		confirm_address()
		check_address()
		return

def apply_in_r2():
	global target_binary
	global address
	global hex_data

	clear_screen()
	r2 = r2pipe.open(f"{target_binary}")
	r2.cmd(f"s {address}")
	r2.cmd("oo+")
	r2.cmd(f"wx {hex_data}@{address}")

	print("Finished commands to write, please check output...")
	sleep(SLEEP_TIME)
	print(r2.cmd("px"))
	pause()
	return

def main():
	user_input = ""
	clear_screen()
	print("What would you like to do?")
	print(f"1) Compile to executable code\t\tIn File: {in_file if in_file else False}")
	print(f"2) Grab .text section from binary code\tOut File: {out_file if out_file else False}")
	print(f"3) Check Hex of .text section\t\tHex Data: {hex_data if hex_data else False}")
	print(f"4) Check address \t\t\tAddress: {address if address else False}")
	print(f"5) Push code to r2\t\t\tTarget Binary: {target_binary if target_binary else False}")
	print("6) Exit")
	user_input = input(">")

	if (user_input == "1"):
		compile_file()
		main()
	elif (user_input == "2"):
		get_hex_data()
		main()
	elif (user_input == "3"):
		read_hex_data()
		main()
	elif (user_input == "4"):
		check_address()
		main()
	elif (user_input == "5"):
		apply_in_r2()
		main()
	elif (user_input == "6"):
		print("Exiting!")
		exit(0)
	else:
		print("Invalid input! Please enter a number between 1 and 6!")
		sleep(2)
		main()


if __name__ == "__main__":
	parser = argparse.ArgumentParser(description="A way to shove C code into a binary")
	parser.add_argument("-in", "--in-file", help="This is the raw .c file")
	parser.add_argument("-out", "--out-file", help="This is the .out file")
	parser.add_argument("-t", "--target-binary", help="This is the target binary")

	args = parser.parse_args()
	print(args)
	if (args.in_file):
		in_file = args.in_file
	if (args.out_file):
		out_file = args.out_file
	if (args.target_binary):
		target_binary = args.target_binary
	
	if (not in_file or not target_binary):
		print("You need to specify at least the in_file and target_binary arguments!")
		parser.print_help()
		exit(-1)
	else:
		main()
