'use strict';

const express = require('express');
const morgan = require('morgan');
const bodyParser = require('body-parser');
const multer = require('multer');
const path = require('path');
const DBAbstraction = require('./DBAbstraction');
const fs = require('fs');

const app = express();
const upload = multer({ dest: 'uploads/' }); // Specify the destination folder for uploaded files

// const filePathPosts = path.join('/home/pi', 'Bifrost', 'posts.sqlite'); // For server use
const filePathPosts = path.join(__dirname, 'data', 'posts.sqlite'); // For local use
const dbPosts = new DBAbstraction(filePathPosts);

app.use(morgan('dev'));
app.use(express.static('public'));
app.use(bodyParser.urlencoded({ extended: false }));
app.use(bodyParser.json());

app.get('/', async (req, res) => {
    try {
        const allPosts = await dbPosts.getAll();
        res.json(allPosts);
    } catch (error) {
        console.error(error);
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

app.post('/posts', upload.single('PostFile'), async (req, res) => {
    try {
        const userText = req.body.PostTextArea;
        const userFile = req.file;

        console.log('userFile:', userFile);
        console.log('generateUniqueFilePath - userFile:', generateUniqueFilePath(userFile));

        const filePath = generateUniqueFilePath(userFile);

        console.log('filePath:', filePath);

        await moveFileToDestination(userFile.path, filePath);

        const newPost = {
            PostTextArea: userText,
            FilePath: filePath,
        };

        await dbPosts.insertPost(newPost.PostTextArea, newPost.FilePath);

        const allPosts = await dbPosts.getAll();
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
    .then(async () => {
        app.listen(53140, () => console.log('The server is up and running...'));

        // //Testing inserting both text and a file(path)
        // const testText = 'test text';
        // const testFilePath = path.join(__dirname, 'testfile.txt');

        // try {
        //     // Insert the test post with the file path into the database
        //     await dbPosts.insertPost(testText, testFilePath);
        //     console.log('Test post inserted successfully');
        // } catch (error) {
        //     console.error('Error inserting test post:', error);
        // }
    })
    .catch(err => {
        console.log('Problem setting up the database');
        console.log(err);
    });

function getFileExtension(fileNameOrPath) {
    if (typeof fileNameOrPath !== 'string') {
        return '';
    }
    const parts = fileNameOrPath.split('.');
    if (parts.length === 1) {
        return '';
    }
    return parts.pop().toLowerCase();
}

function generateUniqueFilePath(userFile) {
    const timestamp = Date.now();
    const randomNumber = Math.floor(Math.random() * 1000000);
    const fileName = `file_${timestamp}_${randomNumber}`;

    const fileExtension = getFileExtension(userFile.originalname); // Use the originalname property to get the file extension
    const filePath = `uploads/${fileName}${fileExtension ? '.' + fileExtension : ''}`;

    return filePath;
}

function moveFileToDestination(sourcePath, destinationPath) {
    return new Promise((resolve, reject) => {
        fs.copyFile(sourcePath, destinationPath, (err) => {
            if (err) {
                reject(err);
            } else {
                resolve();
            }
        });
    });
}
