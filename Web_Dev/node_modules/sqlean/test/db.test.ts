import { Db } from "../src/Db";

Db.Open('test/data.db');

//
// SELECT
//

/*let cmtv = Db.Select.Get({
    table: 'customer',
    columns: '*',
    where: ['@customerId', '=', 'CMTV']
});

console.log(cmtv);*/

let customers = Db.Select.All({
    table: 'customer',
    columns: ['firstName', 'age']
});

console.log(customers);