# Simple Unix Shell Implementation

This is a basic program that simulates a shell which can execute just about any program. 

To start, simply run the make file to create the shell executable and then run the generated executable. 


## Usage:

# View command history	

	hist

# Quit the terminal		

	quit

# Pipe output to process 					(example)	

	ls -al | grep Oct | wc

# Output to a file 						(example)		
	
	ls -al | grep Oct | wc > o.txt

# Execute in background	 					(example)	

	ls -al | grep Oct | wc > o.txt &


## Author

- **Name:** Thomas Pelegrin
