
In order to avoid adding the entire FreeRTOS source to every project
i.e., copies of all of the files, instead the MPLAB project properties
is set to include the source directories in the compilation path, etc.

For this to work, all of those paths must be set correctly for the 
particular file structure.

The easiest way to do this is to make a copy of the entire "rd1_jim" folder
that is under the "Projects" folder. You can then give the folder a new 
name, such as rd2. After that, you can use MPLAB to open the MPLAB project
that is underneath this new folder, give it a new name, and add/remove
files from the project as needed.

Saves tons of time (and possible errors!) typing in horrendous relative
paths.

Dr. J 