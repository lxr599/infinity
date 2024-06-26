import time

import infinity.index as index
import pytest
import random
import pandas
from threading import Thread, Lock
from infinity.common import ConflictType
from infinity.errors import ErrorCode
from infinity.connection_pool import ConnectionPool
from infinity.db import Database
from infinity.remote_thrift.infinity import RemoteThriftInfinityConnection

max_count = 500 * 2000
lock = Lock()
deleting_list = []
kNumThread = 8

class TestInsertDeleteParallelSimple:
    @pytest.mark.skip(reason="segment fault")
    def test_insert_and_delete_parallel_simple(self, get_infinity_connection_pool):
        connection_pool = get_infinity_connection_pool
        infinity_obj = connection_pool.get_conn()
        db_obj = infinity_obj.get_database("default")
        res = db_obj.drop_table("insert_delete_test", ConflictType.Ignore)
        assert res.error_code == ErrorCode.OK
        table_obj = db_obj.create_table("insert_delete_test", {
            "id": "int64"}, ConflictType.Error)
        connection_pool.release_conn(infinity_obj)

        count_num = [0]
        threads = []
        for i in range(kNumThread):
            threads.append(Thread(target=worker_thread, args=[connection_pool, count_num, i]))
        for i in range(len(threads)):
            threads[i].start()
        for i in range(len(threads)):
            threads[i].join()

        infinity_obj = connection_pool.get_conn()
        db_obj = infinity_obj.get_database("default")
        table_obj = db_obj.get_table("insert_delete_test")
        res = table_obj.output(['*']).to_df()
        print(res)
        assert len(res) == 0


def worker_thread(connection_pool: ConnectionPool, count_num, thread_id):
    infinity_obj = connection_pool.get_conn()
    db_obj = infinity_obj.get_database("default")
    table_obj = db_obj.get_table("insert_delete_test")
    while (True):
        choich = random.randint(0, 1)
        lock.acquire()
        if (count_num[0] < max_count and random.randint(0, 1) == 0):
            print("insert")
            print(count_num[0])
            count_num[0] += 500
            lock.release()
            value = []
            for i in range(count_num[0] - 500, count_num[0]):
                value.append({"id": i})
            table_obj.insert(value)
            lock.acquire()
            deleting_list.append(count_num[0] - 500)
            lock.release()
        else:
            if (len(deleting_list) == 0):
                lock.release()
                if (count_num[0] == max_count):
                    break
                else:
                    continue
            else:
                print("delete")
                delete_index = random.randint(0, len(deleting_list) - 1)
                detele_id = deleting_list[delete_index]
                deleting_list.pop(delete_index)
                lock.release()
                try:
                    table_obj.delete(f"id > {detele_id - 1} and id < {detele_id + 500}")
                except Exception as e:
                    print(e)

    connection_pool.release_conn(infinity_obj)
