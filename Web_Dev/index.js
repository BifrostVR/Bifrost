'use strict' 
 
const express = require('express');
const morgan = require('morgan'); 
const bodyParser = require("body-parser"); 
const DBAbstraction = require('./DBAbstraction'); 
const path = require('path');

const filePathPosts =  path.join(__dirname , 'data', 'posts.sqlite');
const dbPosts = new DBAbstraction(filePathPosts);

const app = express(); 

app.use(morgan('dev')); 
app.use(express.static('public'));
app.use(bodyParser.urlencoded({ extended: false })); 
app.use(bodyParser.json());

app.get('/', async (req, res) => { 
    const allPosts = await dbPosts.getAll(); 
    res.render('allPosts', {games: allPosts});
});

app.post('/', async (req, res) => { 
    const userText = req.body.userText;
    const userFile = req.body.userFile;
    await dbPosts.insertPost(userText, userFile);
    const all = await dbPosts.getAll();
    console.log(all);
    res.json(all);
});

app.use((req, res) => { 
    res.status(404).send(`<h2>Uh Oh!</h2><p>Sorry ${req.url} cannot be found here</p>`); 
});

dbPosts.init() 
    .then(() => { 
        app.listen(53140, () => console.log('The server is up and running...')); 
        dbPosts.insertPost("test text", "test file");
    }) 
    .catch(err => { 
        console.log('Problem setting up the database'); 
        console.log(err); 
    });