'use strict';

const express = require('express');
const morgan = require('morgan');
const bodyParser = require('body-parser');
const DBAbstraction = require('./DBAbstraction');
const path = require('path');
const start = performance.now();

// const filePathPosts = path.join('/home/pi', 'Bifrost', 'posts.sqlite'); // For server use
const filePathPosts = path.join(__dirname, 'data', 'posts.sqlite'); // For local use
const dbPosts = new DBAbstraction(filePathPosts);

const app = express();

app.use(morgan('dev'));
app.use(express.static('public'));
app.use(bodyParser.urlencoded({ extended: false }));
app.use(bodyParser.json());

app.get('/', async (req, res) => {
    try {
        const end = performance.now();
        const responseTime = end - start;
        console.log(`Response time: ${responseTime} milliseconds`);
        const allPosts = await dbPosts.getAll();
        const renderedHTML = await ejs.renderFile('views/allPostsView.ejs', { posts: allPosts });
        res.render('allPosts', { posts: allPosts, renderedHTML: renderedHTML });
    } catch (err) {
        console.error(err);
        res.status(500).send('Internal Server Error');
    }
});

app.get('/posts', async (req, res) => {
    try {
        const allPosts = await dbPosts.getAll();
        res.json(allPosts);
    } catch (error) {
        console.error(error);
        res.status(500).send('Internal Server Error');
    }
});

app.post('/posts', async (req, res) => {
    try {
        const userText = req.body.PostTextArea;
        const userFile = req.body.PostFile;
        await dbPosts.insertPost(userText, userFile);
        const allPosts = await dbPosts.getAll();
        console.log(allPosts);
        res.json(allPosts);
    } catch (error) {
        console.error(error);
        res.status(500).send('Internal Server Error');
    }
});
  

app.use((req, res) => {
  res.status(404).send(`<h2>Uh Oh!</h2><p>Sorry ${req.url} cannot be found here</p>`);
});

dbPosts.init()
  .then(() => {
    app.listen(53140, () => console.log('The server is up and running...'));
    dbPosts.insertPost('test text', 'test file');
  })
  .catch(err => {
    console.log('Problem setting up the database');
    console.log(err);
  });
