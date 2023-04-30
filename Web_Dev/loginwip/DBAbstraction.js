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
        const sql = ` 
            CREATE TABLE IF NOT EXISTS 'Userpass' (  
                'Id' INTEGER,  
                'Username' TEXT,  
                'Password' TEXT,  
                PRIMARY KEY('Id') 
            );             
        `; 

        //
 
        return new Promise((resolve, reject) => { 
            this.db.run(sql, [], (err) => {                 
                if(err) { 
                    reject(err); 
                } else { 
                    resolve(); 
                } 
            }); 
        }); 
    }

    insertUserpass(username, password)
    {
        const sql = 'INSERT INTO Userpass (Username, Password) VALUES (?, ?);'; 
        return new Promise((resolve, reject) => { 
            this.db.run(sql, [username, password], (err) => {                 
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
        const sql = ` 
            SELECT Username, Password 
            FROM Userpass 
        `; 
 
        return new Promise((resolve, reject) => { 
            this.db.all(sql, [], (err, rows) => {                 
                if(err) { 
                    reject(err); 
                } else { 
                    resolve(rows); 
                } 
            }); 
        }); 
    } 
    }
 
    // insertLegend(firstName, lastName, birthDate) { 
    //     const sql = 'INSERT INTO CSLegends (FirstName, LastName, BirthDate) VALUES (?, ?, ?);'; 
    //     return new Promise((resolve, reject) => { 
    //         this.db.run(sql, [firstName, lastName, birthDate], (err) => {                 
    //             if(err) { 
    //                 reject(err); 
    //             } else { 
    //                 resolve(); 
    //             } 
    //         }); 
    //     }); 
    // } 
 
    // getLegendByLastName(lastName) { 
    //     const sql = ` 
    //         SELECT FirstName, LastName, BirthDate 
    //         FROM CSLegends 
    //         WHERE LastName = ? COLLATE NOCASE; 
    //     `; 
    //     return new Promise((resolve, reject) => { 
    //         this.db.get(sql, [lastName], (err, row) => {                 
    //             if(err) { 
    //                 reject(err); 
    //             } else { 
    //                 resolve(row); 
    //             } 
    //         }); 
    //     }); 
    // } 
     
    // getAllLegendsBornOnOrAfter(year) { 
         
    //     if(!Number.isInteger(year)) { 
    //         return null; 
    //     } 
 
//         const sql = ` 
//             SELECT FirstName, LastName, BirthDate 
//             FROM CSLegends 
//             WHERE BirthDate >= ?; 
//         `; 
 
//         return new Promise((resolve, reject) => { 
//             this.db.all(sql, [`${year}-01-01`], (err, rows) => {                 
//                 if(err) { 
//                     reject(err); 
//                 } else { 
//                     resolve(rows); 
//                 } 
//             }); 
//         }); 
//     } 
// } 

module.exports = DBAbstraction;