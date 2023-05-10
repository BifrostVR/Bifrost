const sqlite3 = require('sqlite3'); 
 
class DBAbstraction { 
    constructor(fileName) { 
        this.fileName = fileName; 
    } 
 
    init() { 
        return new Promise((resolve, reject) => { 
            this.db = new sqlite3.Database(this.fileName, async (err) => 
            { 
                if(err) 
                { 
                    reject(err); 
                } 
                else 
                { 
                    try 
                    { 
                        await this.createTables(); 
                        resolve(); 
                    } 
                    catch (err) 
                    { 
                        reject(err); 
                    } 
                } 
            }); 
        }); 
    } 
 
    createTables() { 
        const sqlposts = ` 
            CREATE TABLE IF NOT EXISTS 'Posts' (  
                'Id' INT AUTO_INCREMENT,  
                'PostTextArea' TEXT,  
                'PostFile' TEXT,
                PRIMARY KEY('Id') 
            );             
        `;

        return new Promise((resolve, reject) => { 
            this.db.run(sqlposts, [], (err) => {                 
                if(err) { 
                    reject(err); 
                } else { 
                    resolve(); 
                } 
            }); 
        }); 
    }

    insertPost(userText, userFile)
    {
        const sqlposts = 'INSERT INTO Posts (PostTextArea, PostFile) VALUES (?, ?);'; 
        return new Promise((resolve, reject) => { 
            this.db.run(sqlposts, [userText, userFile], (err) => {                 
                if(err) { 
                    reject(err); 
                } else { 
                    resolve(); 
                } 
            }); 
        });  
    }

    getAll()
    {
        const sqlPosts = ` 
            SELECT PostTextArea, PostFile 
            FROM Posts 
        `; 
 
        return new Promise((resolve, reject) => { 
            this.db.all(sqlPosts, [], (err, rows) => {                 
                if(err) { 
                    reject(err); 
                } else { 
                    resolve(rows); 
                } 
            }); 
        }); 
    } 
    }
module.exports = DBAbstraction;