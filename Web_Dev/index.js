'use strict';

const express = require('express');
const morgan = require('morgan');
const bodyParser = require('body-parser');
const multer = require('multer');
const path = require('path');
const DBAbstraction = require('./DBAbstraction');
const { generateUniqueFilePath, moveFileToDestination } = require('./utils');
const fs = require('fs').promises;

const app = express();
const upload = multer({ dest: 'uploads/' }); // Specify the destination folder for uploaded files

const filePathPosts = path.resolve(__dirname, 'data', 'posts.sqlite');
const dbPosts = new DBAbstraction(filePathPosts);

app.use(morgan('dev'));
app.use(express.static('public'));
app.use(bodyParser.urlencoded({ extended: false }));
app.use(bodyParser.json());

const sendErrorResponse = (res, error) => {
    console.error(error);
    res.status(500).send('Internal Server Error');
};

const handlePostRequest = async (req, res) => {
    try {
        const postTextArea = req.body.PostTextArea;
        const file = req.file;

        let filePath = null;

        if (file) {
            // Generate a unique file name or create a suitable file path/URL
            filePath = generateUniqueFilePath(file);

            // Move or copy the file to the desired location (e.g., a folder on the server or a cloud storage)
            await moveFileToDestination(file, filePath);
        }

        // Insert the post into the database with the file path or null
        await dbPosts.insertPost(postTextArea, filePath);

        // Send a response indicating success
        res.json({ success: true });
    } catch (error) {
        // Handle any errors that occurred during the file upload or database insertion
        sendErrorResponse(res, error);
    }
};

app.get('/', async (req, res) => {
    try {
        const allPosts = await dbPosts.getAll();
        res.json(allPosts);
    } catch (error) {
        sendErrorResponse(res, error);
    }
});

app.get('/posts', async (req, res) => {
    try {
        const allPosts = await dbPosts.getAll();
        res.json(allPosts);
    } catch (error) {
        sendErrorResponse(res, error);
    }
});

app.post('/posts', (req, res, next) => {
    upload.single('PostFile')(req, res, function (err) {
        if (err) {
            // Handle multer errors
            console.error(err);
            res.status(400).json({ success: false, error: 'File upload error.' });
        } else {
            next(); // Continue to process the request
        }
    });
}, handlePostRequest);

app.get('/uploads/:filename', async (req, res) => {
    try {
        const filename = req.params.filename;
        const filePath = path.resolve(__dirname, 'uploads', filename);

        await fs.access(filePath, fs.constants.F_OK);
        res.sendFile(filePath);
    } catch (error) {
        console.error(error);
        res.status(404).send(`<h2>Uh Oh!</h2><p>Sorry /uploads/${filename} cannot be found here</p>`);
    }
});

app.use((req, res) => {
    res.status(404).send(`<h2>Uh Oh!</h2><p>Sorry ${req.url} cannot be found here</p>`);
});

const initializeServer = async () => {
    try {
        await dbPosts.init();
        app.listen(53140, () => console.log('The server is up and running...'));
    } catch (err) {
        console.log('Problem setting up the database');
        console.log(err);
    }
};

initializeServer();