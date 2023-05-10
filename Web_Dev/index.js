'use strict' 
 
const express = require('express');
const morgan = require('morgan'); 
const bodyParser = require("body-parser"); 
const DBAbstraction = require('./DBAbstraction'); 
const path = require('path');

const filePathGames =  path.join(__dirname , 'data', 'games.sqlite');
const filePathPosts =  path.join(__dirname , 'data', 'posts.sqlite');
const dbGames = new DBAbstraction(filePathGames);
const dbPosts = new DBAbstraction(filePathPosts);

const app = express(); 

app.use(morgan('dev')); 
app.use(express.static('public'));
app.use(bodyParser.urlencoded({ extended: false })); 
app.use(bodyParser.json());

app.get('/', async (req, res) => { 
    const all = await dbGames.getAll(); 
    res.render('all', {legends: all}); 
});

app.post('/', async (req, res) => { 
    const team1Name = req.body.team1Name;
    const team1Score = req.body.team1Score;
    await dbGames.insertGame(team1Name, team1Score);
    const all = await dbGames.getAll();
    console.log(all);
    res.json(all);
});

app.use((req, res) => { 
    res.status(404).send(`<h2>Uh Oh!</h2><p>Sorry ${req.url} cannot be found here</p>`); 
});

dbGames.init() 
    .then(() => { 
        app.listen(53140, () => console.log('The server is up and running...')); 
        dbGames.insertGame("test", "test2");
    }) 
    .catch(err => { 
        console.log('Problem setting up the database'); 
        console.log(err); 
    });