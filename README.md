1) This project is going to be a project to remotely controll someone else's neovim cursor and share contents of their screen for paired programming exercises!

Roadmap
4) Start to open a buffer and populate contents

Todos
1) Make a function to replace the entire buffer contents
2) Figure out a way to listen to file changes in c maybe?
    * set up an autocmd with "run lua" and a function string?
    * another way?
3) If this is possible, make a POC with a "client" side
4) In order to launch the plugin, I am going to need to make a small portion of lua code which calls the app with the current win ID and stuff

