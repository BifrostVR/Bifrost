'use strict' 
 
const express = require('express');
const handlebars = require('express-handlebars').create({defaultLayout: 'main'});  
const morgan = require('morgan'); 
const bodyParser = require("body-parser"); 
const DBAbstraction = require('./DBAbstraction'); 
const path = require('path');

const filePath =  path.join(__dirname , 'data', 'userpass.sqlite');
const db = new DBAbstraction(filePath);
 
const app = express(); 

app.engine('handlebars', handlebars.engine); 
app.set('view engine', 'handlebars');
app.use(morgan('dev')); 
app.use(express.static('public'));
app.use(bodyParser.urlencoded({ extended: false })); 
app.use(bodyParser.json());

app.get('/', async (req, res) => { 
    const all = await db.getAll(); 
    res.render('all', {legends: all}); 
});

app.post('/', async (req, res) => { 
    const username = req.body.username;
    const password = req.body.password;
    await db.insertUserpass(username, password);
    const all = await db.getAll();
    console.log(all);
    res.json(all);
});

app.use((req, res) => { 
    res.status(404).send(`<h2>Uh Oh!</h2><p>Sorry ${req.url} cannot be found here</p>`); 
});

db.init() 
    .then(() => { 
        app.listen(53140, () => console.log('The server is up and running...')); 
        db.insertUserpass("test1", "test2");
        db.insertUserpass("test3", "test4");
    }) 
    .catch(err => { 
        console.log('Problem setting up the database'); 
        console.log(err); 
    });

 
