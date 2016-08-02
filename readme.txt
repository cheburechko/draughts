Game of Draughts made by Cherenkov Pavel, 
May 2012, NSU Physics Facility 1st course.

Installation:

The game needs ncurses v5.9 installed on your computer.
To compile use:
	make draughts
Run "draughts" to play the game.

Description:

This is a game of draughts with modifiable rules and AI bot.

To start new game, choose "New Game" option.
You can choose easy/medium/hard bots or human control for
each side. You can control your pieces with arrow keys and enter,
A game continues until one of the player does not have any moves
or there were done 500 moves total.

To change game rules, choose "Choose Rules" option.
Rules must be written in .txt files and stored to in the 
game directory. If an error occurs during reading rules,
rules will be reset to default (russian checkers).

To know more about the rules, choose "Rules" option.
Note that not all rules provide a description with them,
and the rules description remains from previous rules.

To know about modifing rules, read rules.txt.
To know more about technical things, read tech.txt.
