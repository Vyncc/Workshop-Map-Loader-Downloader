This was my first ever c++ project. I learned c++ while I was developing it so the code is pretty bad not gonna lie.

# Workshop Map Loader & Downloader

This plugin allows you :

- To load Workshop Maps from a folder. 

 ![Map Loader Tab](https://i.postimg.cc/bYMgjV2C/1ere-Partie.png)


- To search a map on the steam workshop directly on the plugin interface.

 ![Search Workshop Tab](https://i.postimg.cc/VNYBF336/2eme-Partie.png)


- To download a map from an url, or, from the searching results into your maps folder

### Additional Informations

- This plugin can be used by Epic Games users.

- Languages : English or French.

- Add a bind to open the plugin window :
    - **F2->bindings**, add a bind with this command line : `"togglemenu WorkshopMapLoaderMenu"`
    - **F2->plugins->WorkshopMapPlugin**, here you just need to put a Key in the text field, and press "Set Bind".


## Tutorial

You have to :
- Create a folder wherever you want on your computer that will contain the maps
- Open the plugin
- Copy/paste the folder's path into the text field (in the "Map Loader" tab)
- Go in "Search Workshop" tab and search a workshop
    - Put some keywords (ex : Dribbling Challenge, Rings etc...) and press "Search". The results will be displayed below.
- Download a map
    - From The Searching Results :
        - Do a research
        - Click on "Download Map"
    - With Url :
        - Put a Steam workshop url in the text field
        - Click on "Download".
    - Manually :
        - Download a map with [steamworkshopdownloader.io](https://steamworkshopdownloader.io/), create a new folder in your maps folder, then extract the files of the map into this new folder
- Click on "Refresh Maps"
- Choose the map you want to play


I've made a tutorial if you are struggling to make it work : [https://www.youtube.com/watch?v=mI2PqkissiQ](https://www.youtube.com/watch?v=mI2PqkissiQ)


## Bugs/Issues Known

- If you use Windows 7 you wont be able to load the plugin, add me on discord (just below) I can give you an old version working for Windows 7

- Mutators are not working for clients in a multiplayer server

- If the path you put contains "//" just after the disk name (ex : C://RL maps/), you wont be able to load a map
    - Only put one "/"

- If your game crashes when you click on download
    - This is probably because the place where you have created the maps folder need administrator permission

- If there is accents, arabic or special characters in the path, it tells you "The directory you entered doesn't exist"
    - You have to find a path that doesn't contain these characters .

- If your game crashes when clicking on "Refresh Maps"
    - Maybe because your maps folder contains other things, you need to dedicate the maps folder only for the workshops maps and nothing else
