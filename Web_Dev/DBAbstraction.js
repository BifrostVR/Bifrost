const sqlite3 = require('sqlite3');
const fs = require('fs');

class DBAbstraction {
    constructor(fileName) {
        this.fileName = fileName;
    }

    init() {
        return new Promise((resolve, reject) => {
            this.db = new sqlite3.Database(this.fileName, async (err) => {
                if (err) {
                    reject(err);
                }
                else {
                    try {
                        await this.createTables();
                        resolve();
                    }
                    catch (err) {
                        reject(err);
                    }
                }
            });
        });
    }

    createTables() {
        const sqlPosts = ` 
        CREATE TABLE IF NOT EXISTS Posts (
            Id INTEGER PRIMARY KEY AUTOINCREMENT,
            PostTextArea TEXT,
            FilePath TEXT
          );           
        `;

        return new Promise((resolve, reject) => {
            this.db.run(sqlPosts, [], (err) => {
                if (err) {
                    reject(err);
                } else {
                    resolve();
                }
            });
        });
    }

    insertPost(userText, userFile) {
        return new Promise((resolve, reject) => {
            // Generate a unique file name or create a suitable file path/URL
            console.log('userFile:', userFile);
            const filePath = generateUniqueFilePath(userFile); // Pass the userFile parameter to generateUniqueFilePath

            // Move or copy the file to the desired location (e.g., a folder on the server or a cloud storage)
            moveFileToDestination(userFile, filePath)
                .then(() => {
                    // Insert the post into the database with the file path or URL
                    const sqlposts = 'INSERT INTO Posts (PostTextArea, FilePath) VALUES (?, ?);';
                    this.db.run(sqlposts, [userText, filePath], (err) => {
                        if (err) {
                            reject(err);
                        } else {
                            resolve();
                        }
                    });
                })
                .catch((err) => {
                    reject(err);
                });
        });
    }


    getAll() {
        const sqlPosts = ` 
            SELECT PostTextArea, FilePath 
            FROM Posts 
        `;

        return new Promise((resolve, reject) => {
            this.db.all(sqlPosts, [], (err, rows) => {
                if (err) {
                    reject(err);
                } else {
                    resolve(rows);
                }
            });
        });
    }
}

function generateUniqueFilePath(userFile) {
    // Generate a unique file name based on a timestamp and random number
    console.log('generateUniqueFilePath - userFile:', userFile);
    const timestamp = Date.now();
    const randomNumber = Math.floor(Math.random() * 1000000);
    const fileName = `file_${timestamp}_${randomNumber}`;

    // Append the desired file extension based on the file type
    const fileExtension = getFileExtension(userFile); // Replace with your logic to get the file extension
    const filePath = `uploads/${fileName}.${fileExtension}`;

    return filePath;
}


function getFileExtension(userFile) {
    if (typeof userFile === 'string') {
        // Extract the file extension from the file name
        const parts = userFile.split('.');
        if (parts.length === 1) {
            return ''; // No file extension found
        }
        return parts.pop().toLowerCase();
    } else if (userFile instanceof File) {
        // Extract the file extension from the file object
        return userFile.name.split('.').pop().toLowerCase();
    }

    return ''; // Invalid input or no file extension found
}

function moveFileToDestination(sourcePath, destinationPath) {
    return new Promise((resolve, reject) => {
        fs.rename(sourcePath, destinationPath, (err) => {
            if (err) {
                reject(err);
            } else {
                resolve();
            }
        });
    });
}


module.exports = DBAbstraction;