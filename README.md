# PongBreak #
##### If Pong and BrickBreak had a baby. AN ACTION-PACKED BABY! #####

[David Hartman](https://twitter.com/wednesdayscones) and [Ty Wood](http://tyskwo.com)
Repository URL: https://tyskwo@bitbucket.org/tyskwo/egp405_a2_02_hartman_wood.git

* * * 

### Project Synopsis ###
##### A reliable multiplayer experience. #####

The goal of this project was to create a Client-Server model multiplayer game using [RakNet](http://www.jenkinssoftware.com). Along with basic networking, PongBreak also features interpolation and dead reckoning of both the paddles and the ball to allow for smoother (perceived) gameplay. Each server is theoretically capable of running 8 games simultaneously (we've run two at once for proof-of-concept), though it has not been stress tested.

* * *

### Gameplay ###
##### Combining elements of Pong and Breakout. #####

The goal scoring of Pong with the brick breaking of Breakout. Use the grid to better estimate where the ball is going to be!

Scoring works with each brick hit is one point, and each time the ball hits the back wall is three points. The first to fifty points wins! The catch is that the board slowly rotates back and forth, and the ball can get stuck behind the bricks!

##### Controls: #####
UP and DOWN arrow keys to move your paddle up and down. Player two hits space to start the game.