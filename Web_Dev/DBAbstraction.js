const sqlite3 = require('sqlite3');
const path = require('path');
const { generateUniqueFilePath, getFileExtension, moveFileToDestination } = require('./utils');

class DBAbstraction {
    constructor(fileName) {
        this.fileName = fileName;
    }

    init() {
        return new Promise((resolve, reject) => {
            this.db = new sqlite3.Database(this.fileName, async (err) => {
                if (err) {
                    reject(err);
                } else {
                    try {
                        await this.createTables();
                        resolve();
                    } catch (err) {
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
            // Insert the post into the database with the file path or URL
            const sqlposts = 'INSERT INTO Posts (PostTextArea, FilePath) VALUES (?, ?);';
            this.db.run(sqlposts, [userText, userFile], (err) => {
                if (err) {
                    reject(err);
                } else {
                    resolve();
                }
            });
        });
    }

    getPostByFilePath(filePath) {
        const sqlPostByFilePath = `
          SELECT *
          FROM Posts
          WHERE FilePath = ?`;

        return new Promise((resolve, reject) => {
            this.db.get(sqlPostByFilePath, [filePath], (err, row) => {
                if (err) {
                    reject(err);
                } else {
                    resolve(row);
                }
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

module.exports = DBAbstraction;