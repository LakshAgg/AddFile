# AddFile
Just a simple and stupid program to create source files with some prefilled data.

## Config
The configuration can be stored anywhere u like. On running the program for the first time it will ask for the path to the config file.

### Config File
The file follows a really simple syntax.
```
Variables and Comments

space if u want


--file extension
content for given file extension
-- (to mark the end)


--.c
Content for .c file
--


--.h
content for .h file
--


--.html
content for .html file
--

```

#### Variables and Comments

The variables can contain any character except ` ` space and `#`. Even % is a valid name. A variable differs from a comment by adding an = sign after the name. On adding a space before =, the whole line is considered a comment. Anything after the = will be assigned as the value.

Comments can also be created with a # in the beginning of the line

Eg.
```
Name=     this is value with spaces
Author=Insane
Variable=Value

Yeah this is a comment now even after an=equals_sign
This is=comment
Yay a comment
#this=comment
# this is a comment
```

#### File Contents
Data for each file extension is started with --[file_name_ends_with][new_line]. Then mark the end of the data with --[new_line].

In order to insert variables use $Variable_name. In order to add a space after variable use double spaces. In order to use -- or $ in the content, escape them. Eg. `\--` and `\$`;

## Program
The Usage is `AddFile file_name ext1 ext2 ... [options]`. Eg.`AddFile Test .c .h .html .js`

### Options
* -d => to create a directory with the same file_name and put the source file under it.
* -v [Variable_Name] [Variable Value] => this just assigns or overwrites the variable given in the config.