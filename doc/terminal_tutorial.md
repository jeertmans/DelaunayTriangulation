Using the Terminal
==================

## Basics

[Here](https://medium.com/the-code-review/top-10-bash-file-system-commands-you-cant-live-without-4cd937bd7df1) is a description of the 10 most usefull commands when using the terminal.

In short:

 1. **ls**:
     * **l**i**s**t files
 2. **cd**:
     * **c**hange **d**irectory
 3. **mkdir**:
     * **m**a**k**e **dir**ectory
 4. **touch**:
     * do *as if* you modified a file or create an empty file if the file does not exist
 5. **cat**:
     * con**cat**enate and print files to the standard output (stdout)
 6. **mv**:
     * **m**o**v**e files
 7. **cp**:
     * **c**o**p**y files
 8. **rm**:
     * **r**e**m**ove files
 9. **chmod**:
     * **ch**ange **mod**e (not really usefull in this case...)
 10. **man**:
     * display the **man**uals for a command/program/function

## Completion

You can autocomplete most commands by pressing *Tab ↹*.
If it doesn't work, it means that there are multiple possibilities for autocompletion.
Pressing *Tab ↹* a second time should display all possibilities.

**Life Pro Tips :**
Press *Tab ↹* regularly to make sure you don't mess anything up. Indeed, if you press it twice and nothing shows up, it often mean that you screwed up...

## Exercice

Now, you should be able to follow those steps easily using above commands.
After each command you made, verify that you did everything perfectly by using **ls** to list the files.

 * make a directory named **first** and another directory named **second**.
 * go into **first** (change the working directory to **first**)
 * create a file name **new**
 * create a copy of **new**, named **copy**, into **second**
 * go into **second** (change the working directory to **second**)
 * remove **copy**
 * remove **first** and **second**