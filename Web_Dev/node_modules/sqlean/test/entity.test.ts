import { Db, Column, Entity, PrimaryKey, Table, DB } from "../src/index";

Db.Open('test/data.db');

export class MyObj
{
    someStr: string;
    someNum: number;
    someArr: string[];
}

@Table('test_table')
class TestEntity extends Entity
{
    @PrimaryKey
    @Column
    testId: string;

    @Column
    obj: MyObj;

    postGet()
    {
        this.obj.someNum = 1337;
    }
}

let testObj = new TestEntity;
    testObj.testId = 'id1';
    testObj.obj = { someStr: 'sdf', someNum: 5, someArr: ['first', 'second'] }

testObj.save();

console.log(TestEntity.getById('id1'));