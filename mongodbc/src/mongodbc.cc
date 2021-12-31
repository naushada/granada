#ifndef __mongodbc_cc__
#define __mongodbc_cc__

#include <vector>
#include <sstream>

#include "mongodbc.h"

Mongodbc::Mongodbc()
{
    mMongoConn = nullptr;
    mInstance = nullptr;
    mURI.clear();
    mdbName.clear();

}

Mongodbc::Mongodbc(std::string uri_str, std::string db_name, std::uint32_t poolSize)
{
    mMongoConn = nullptr;
    mInstance = nullptr;
    mMongoConnPool = nullptr;
    mURI = uri_str;
    mdbName = db_name;

    mInstance = new mongocxx::instance();
    do {
        if(nullptr == mInstance) {
            ACE_ERROR((LM_ERROR, ACE_TEXT("%D [Master:%t] %M %N:%l instantiation of mongocxx::instance is failed\n")));
            break;
        }

        if(!poolSize) {

            mongocxx::uri uri(uri_str.c_str());
            mMongoConn = new mongocxx::client(uri);

            if(nullptr == mMongoConn) {
                break;
            }
            mMongoUri = mMongoConn->uri();
            mMongoDbInst = mMongoConn->database(db_name.c_str());

            /* Populating the collection Array */
            mMongoCollections[static_cast<size_t>(CollectionName::SHIPPING)] = mMongoDbInst.collection("shipping");
            mMongoCollections[static_cast<size_t>(CollectionName::JOB_SCHEDULING)] = mMongoDbInst.collection("job_scheduling");
            mMongoCollections[static_cast<size_t>(CollectionName::MANIFESTING)] = mMongoDbInst.collection("manifesting");
            mMongoCollections[static_cast<size_t>(CollectionName::SUPPLIES)] = mMongoDbInst.collection("supplies");
            mMongoCollections[static_cast<size_t>(CollectionName::SHIPMENT_PRICING)] = mMongoDbInst.collection("shipment_pricing");
            mMongoCollections[static_cast<size_t>(CollectionName::TRACKING)] = mMongoDbInst.collection("tracking");
            mMongoCollections[static_cast<size_t>(CollectionName::REPORTING)] = mMongoDbInst.collection("reporting");
        } else {
            /* pool of connections*/
            std::string poolUri(uri_str);
            /* reference: http://mongocxx.org/mongocxx-v3/connection-pools/ */
            //poolUri += "/?minPoolSize=10&maxPoolSize=" + std::to_string(poolSize);
            poolUri += "&minPoolSize=10&maxPoolSize=" + std::to_string(poolSize);
            mongocxx::uri uri(poolUri.c_str());
            ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [Master:%t] %M %N:%l The URI is %s\n"), poolUri.c_str()));
            mMongoConnPool = new mongocxx::pool(uri);

            if(nullptr == mMongoConnPool) {
                ACE_ERROR((LM_ERROR, ACE_TEXT("%D [Master:%t] %M %N:%l instantiation of MongoConnPool is failed\n")));
                break;
            }
        }

    }while(0);

}

Mongodbc::~Mongodbc()
{
    if(nullptr != mMongoConn) {
        delete mMongoConn;
    }

    if(nullptr != mInstance) {
        delete mInstance;
    }
}

void Mongodbc::dump_document(CollectionName collection)
{

    auto conn = mMongoConnPool->acquire();
    mongocxx::database dbInst = conn->database(get_dbName().c_str());

    switch(collection) {
        case CollectionName::SHIPPING:
        {
            auto collection = dbInst.collection("shipping");
            auto cursor = collection.find({});
            for (auto&& doc : cursor) {
                std::cout << bsoncxx::to_json(doc) << std::endl;
            }
        }
        break;
    }
 

}

std::string Mongodbc::create_shipment(std::string shipmentRecord, std::string projection)
{
    bsoncxx::document::value new_shipment = bsoncxx::from_json(shipmentRecord.c_str());
    if(nullptr != mMongoConn) {
        mMutex.lock();
        bsoncxx::stdx::optional<mongocxx::result::insert_one> result = get_collection(CollectionName::SHIPPING).insert_one(new_shipment.view());
        mMutex.unlock();
    } else {
        auto conn = mMongoConnPool->acquire();
        mongocxx::database dbInst = conn->database(get_dbName().c_str());
        auto collection = dbInst.collection("shipping");
        bsoncxx::stdx::optional<mongocxx::result::insert_one> result = collection.insert_one(new_shipment.view());
        bsoncxx::oid oid = result->inserted_id().get_oid().value;
        std::string JobID = oid.to_string();
        ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [Worker:%t] %M %N:%l The inserted Oid is %s\n"), JobID.c_str()));
        return(JobID);

        /* Get the newly added shipmentNo */
#if 0
        std::string query("{\"_id\" : {\"$oid\": \"");
            query += JobID + "\"}}";
        ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [Worker:%t] %M %N:%l The query is %s\n"), query.c_str()));

        bsoncxx::document::value filter = bsoncxx::from_json(query.c_str());
        mongocxx::options::find opts{};
        bsoncxx::document::view_or_value outputProjection = bsoncxx::from_json(projection.c_str());
        auto resultFormat = opts.projection(outputProjection);
        mongocxx::v_noabi::cursor cursor = collection.find(filter.view(), resultFormat);
        mongocxx::cursor::iterator iter = cursor.begin();
        bsoncxx::document::view res = *cursor.begin();

        if(iter == cursor.end()) {
            return(std::string());
        }

        std::stringstream rsp("");
        rsp << "{" << bsoncxx::to_json(*iter) << "}";
        return(rsp.str());
#endif
    }

    return(std::string());
}

bool Mongodbc::update_shipment(std::string match, std::string shippingRecord)
{
    bsoncxx::document::value toUpdate = bsoncxx::from_json(shippingRecord.c_str());
    bsoncxx::document::value filter = bsoncxx::from_json(match.c_str());
    if(nullptr != mMongoConn) {
        mMutex.lock();
        //bsoncxx::stdx::optional<mongocxx::result::insert_one> result = get_collection(CollectionName::SHIPPING).insert_one(new_shipment.view());
        mMutex.unlock();
    } else {
        auto conn = mMongoConnPool->acquire();
        mongocxx::database dbInst = conn->database(get_dbName().c_str());
        auto collection = dbInst.collection("shipping");
        //bsoncxx::stdx::optional<mongocxx::result::update> result = collection.update_many(filter.view(), toUpdate.view());

        mongocxx::options::bulk_write bulk_opt;
        mongocxx::write_concern wc;
        bulk_opt.ordered(false);
        wc.acknowledge_level(mongocxx::write_concern::level::k_default);
        bulk_opt.write_concern(wc);
        auto bulk = collection.create_bulk_write(bulk_opt);
        #if 0 
        bsoncxx::document::view filter_view = filter.view();
        auto item = filter_view["shipmentNo"];
        if(item && item.type() == bsoncxx::type::k_array) {
            auto subArr = item.get_array().value;
            for(auto elm : subArr) {
                if(elm.type() == bsoncxx::type::k_utf8) {
                    //auto val = elm.get_utf8().value;
                    auto val = elm.get_utf8().value;
                    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [Worker:%t] %M %N:%l The element is %s\n"), bsoncxx::string::to_string(val).c_str()));
                }
            }
        }
        #endif
        mongocxx::model::update_many upd(filter.view(), toUpdate.view());
        bulk.append(upd);

        auto result = bulk.execute();
        std::int32_t cnt = 0;
        if(result) {
            //cnt = result->updated_count();
            ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [worker:%t] %M %N:%l bulk document updated is %d\n"), cnt));
        }
        //bsoncxx::stdx::optional<mongocxx::result::update> result = collection.update_many(filter.view(), toUpdate.view());
        //bsoncxx::stdx::optional<mongocxx::result::update> result = collection.update_one(filter.view(), toUpdate.view());
        if(result) {
            ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [Worker:%t] %M %N:%l The collection is updated %s\n"), shippingRecord.c_str()));
            return(true);
        } else {
            return(false);
        }
        //bsoncxx::stdx::optional<mongocxx::result::insert_one> result = collection.insert_one(shippingRecord.view());
    }
    #if 0
    bsoncxx::document::value new_shipment = bsoncxx::from_json(shippingRecord.c_str());
    bsoncxx::document::value match = bsoncxx::from_json(match.c_str());
    mMutex.lock();
    bsoncxx::stdx::optional<mongocxx::result::update> result = get_collection(CollectionName::SHIPPING).update_one(match, new_shipment.view());
    mMutex.unlock();
    #endif
    return(true);

}

bool Mongodbc::delete_shipment(std::string doc)
{
    bsoncxx::document::value filter = bsoncxx::from_json(doc.c_str());
    auto conn = mMongoConnPool->acquire();
    mongocxx::database dbInst = conn->database(get_dbName().c_str());
    auto collection = dbInst.collection("shipping");
    //bsoncxx::stdx::optional<mongocxx::result::update> result = collection.update_many(filter.view(), toUpdate.view());

    mongocxx::options::bulk_write bulk_opt;
    mongocxx::write_concern wc;
    bulk_opt.ordered(false);
    wc.acknowledge_level(mongocxx::write_concern::level::k_default);
    bulk_opt.write_concern(wc);
    auto bulk = collection.create_bulk_write(bulk_opt);
    #if 0 
    bsoncxx::document::view filter_view = filter.view();
    auto item = filter_view["shipmentNo"];
    if(item && item.type() == bsoncxx::type::k_array) {
        auto subArr = item.get_array().value;
        for(auto elm : subArr) {
            if(elm.type() == bsoncxx::type::k_utf8) {
                //auto val = elm.get_utf8().value;
                auto val = elm.get_utf8().value;
                ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [Worker:%t] %M %N:%l The element is %s\n"), bsoncxx::string::to_string(val).c_str()));
            }
        }
    }
    #endif
    mongocxx::model::delete_many del(filter.view());
    bulk.append(del);

    auto result = bulk.execute();
    std::int32_t cnt = 0;
    if(result) {
        //cnt = result->updated_count();
        ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [worker:%t] %M %N:%l bulk document deleted is %d\n"), cnt));
    }
    //bsoncxx::stdx::optional<mongocxx::result::update> result = collection.update_many(filter.view(), toUpdate.view());
    //bsoncxx::stdx::optional<mongocxx::result::update> result = collection.update_one(filter.view(), toUpdate.view());
    if(result) {
        ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [Worker:%t] %M %N:%l The shipment from collection is deleted \n")));
        return(true);
    } else {
        return(false);
    }
    //bsoncxx::stdx::optional<mongocxx::result::insert_one> result = collection.insert_one(shippingRecord.view());
    return(true);
}

std::string Mongodbc::get_shipment(std::string collectionName, std::string query, std::string fieldProjection)
{
    std::string json_object;
    bsoncxx::document::value filter = bsoncxx::from_json(query.c_str());
    auto conn = mMongoConnPool->acquire();
    mongocxx::database dbInst = conn->database(get_dbName().c_str());
    auto collection = dbInst.collection(collectionName.c_str());

    mongocxx::options::find opts{};
    bsoncxx::document::view_or_value outputProjection = bsoncxx::from_json(fieldProjection.c_str());
    auto resultFormat = opts.projection(outputProjection);
    mongocxx::v_noabi::cursor cursor = collection.find(filter.view(), resultFormat);
    //bsoncxx::document::view res = *cursor.begin();

    mongocxx::cursor::iterator iter = cursor.begin();

    if(iter == cursor.end()) {
        return(std::string());
    }
    /*
    for (auto&& doc : cursor) {
        ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [worker:%t] %M %N:%l Aero Bill %s\n"), bsoncxx::to_json(doc).c_str()));
    }
    */

    return(std::string(bsoncxx::to_json(*iter).c_str()));

    #if 0
    //bsoncxx::stdx::optional<mongocxx::result::update> result = collection.update_many(filter.view(), toUpdate.view());
    bsoncxx::stdx::optional<mongocxx::cursor> result = collection.find(filter.view());
    bsoncxx::document::value what = bsoncxx::from_json(key.c_str());
    json_object.clear();
    bsoncxx::stdx::optional<bsoncxx::document::value> result = get_collection(CollectionName::SHIPPING).find_one(what);
    json_object =  bsoncxx::to_json(result.view());
    #endif
}

std::string Mongodbc::get_shipmentList(std::string collectionName, std::string query, std::string fieldProjection)
{
    std::string json_object;
    bsoncxx::document::value filter = bsoncxx::from_json(query.c_str());
    auto conn = mMongoConnPool->acquire();
    mongocxx::database dbInst = conn->database(get_dbName().c_str());
    auto collection = dbInst.collection(collectionName.c_str());

    mongocxx::options::find opts{};
    bsoncxx::document::view_or_value outputProjection = bsoncxx::from_json(fieldProjection.c_str());
    auto resultFormat = opts.projection(outputProjection);
    mongocxx::v_noabi::cursor cursor = collection.find(filter.view(), resultFormat);
    mongocxx::cursor::iterator iter = cursor.begin();

    if(iter == cursor.end()) {
        return(std::string());
    }

    std::stringstream result("");
    result << "[";

    for(; iter != cursor.end(); ++iter) {
        result << bsoncxx::to_json(*iter).c_str()
               << ",";
    }

    result.seekp(-1, std::ios_base::end);
    result << "]";

    return(std::string(result.str()));
}

std::string Mongodbc::validate_user(std::string collectionName, std::string query, std::string fieldProjection)
{
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [worker:%t] %M %N:%l Query %s\n"), query.c_str()));
    bsoncxx::document::value filter = bsoncxx::from_json(query.c_str());
    auto conn = mMongoConnPool->acquire();
    mongocxx::database dbInst = conn->database(get_dbName().c_str());
    auto collection = dbInst.collection(collectionName.c_str());
    mongocxx::options::find opts{};
    bsoncxx::document::view_or_value outputProjection = bsoncxx::from_json(fieldProjection.c_str());
    auto resultFormat = opts.projection(outputProjection);
    mongocxx::v_noabi::cursor cursor = collection.find(filter.view(), resultFormat);
    mongocxx::cursor::iterator iter = cursor.begin();
    //bsoncxx::document::view res = *cursor.begin();

    if(iter == cursor.end()) {
        return(std::string());
    }
    /*
    for (auto&& doc : cursor) {
        ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [worker:%t] %M %N:%l validate_user DB Result %s\n"), bsoncxx::to_json(doc).c_str()));
    }
    */

    //return(std::string(bsoncxx::to_json(res).c_str()));
    return(std::string(bsoncxx::to_json(*iter).c_str()));
}

std::string Mongodbc::get_accountInfo(std::string collectionName, std::string query, std::string fieldProjection)
{
    return(validate_user(collectionName, query, fieldProjection));
}

std::string Mongodbc::create_account(std::string accountRecord, std::string projection)
{
    bsoncxx::document::value new_account = bsoncxx::from_json(accountRecord.c_str());
    
    auto conn = mMongoConnPool->acquire();
    mongocxx::database dbInst = conn->database(get_dbName().c_str());
    auto collection = dbInst.collection("account");
    bsoncxx::stdx::optional<mongocxx::result::insert_one> result = collection.insert_one(new_account.view());
    bsoncxx::oid oid = result->inserted_id().get_oid().value;
    std::string JobID = oid.to_string();
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [Worker:%t] %M %N:%l The inserted Oid is %s\n"), JobID.c_str()));
    return(JobID);

#if 0
    std::string query("{\"_id\" : {\"$oid\": \"");
    query += JobID + "\"}}";
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [Worker:%t] %M %N:%l The query is %s\n"), query.c_str()));
    bsoncxx::document::value filter = bsoncxx::from_json(query.c_str());
    mongocxx::options::find opts{};
    bsoncxx::document::view_or_value outputProjection = bsoncxx::from_json(projection.c_str());
    auto resultFormat = opts.projection(outputProjection);
    mongocxx::v_noabi::cursor cursor = collection.find(filter.view(), {});
    //mongocxx::v_noabi::cursor cursor = collection.find(filter.view(), resultFormat);
    mongocxx::cursor::iterator iter = cursor.begin();
    bsoncxx::document::view res = *cursor.begin();

    if(iter == cursor.end()) {
        return(std::string());
    }
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [Worker:%t] %M %N:%l The accountCode is %s \n"), bsoncxx::to_json(res).c_str()));

    std::stringstream rsp("");
    rsp << bsoncxx::to_json(*iter);
    return(rsp.str());
#endif
}

std::string Mongodbc::get_byOID(std::string coll, std::string projection, std::string oid)
{
    auto conn = mMongoConnPool->acquire();
    mongocxx::database dbInst = conn->database(get_dbName().c_str());
    auto collection = dbInst.collection(coll.c_str());

    std::string query("{\"_id\" : {\"$oid\": \"");
    query += oid + "\"}}";

    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [Worker:%t] %M %N:%l The query is %s\n"), query.c_str()));
    bsoncxx::document::value filter = bsoncxx::from_json(query.c_str());
    mongocxx::options::find opts{};
    bsoncxx::document::view_or_value outputProjection = bsoncxx::from_json(projection.c_str());
    auto resultFormat = opts.projection(outputProjection);
    mongocxx::v_noabi::cursor cursor = collection.find(filter.view(), {});
    //mongocxx::v_noabi::cursor cursor = collection.find(filter.view(), resultFormat);
    mongocxx::cursor::iterator iter = cursor.begin();
    bsoncxx::document::view res = *cursor.begin();

    if(iter == cursor.end()) {
        return(std::string());
    }
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [Worker:%t] %M %N:%l The Response is %s \n"), bsoncxx::to_json(res).c_str()));

    std::stringstream rsp("");
    rsp << bsoncxx::to_json(*iter);
    return(rsp.str());

}

std::string Mongodbc::get_documentList(std::string collectionName, std::string query, std::string fieldProjection)
{
    std::string json_object;
    bsoncxx::document::value filter = bsoncxx::from_json(query.c_str());
    auto conn = mMongoConnPool->acquire();
    mongocxx::database dbInst = conn->database(get_dbName().c_str());
    auto collection = dbInst.collection(collectionName.c_str());

    mongocxx::options::find opts{};
    bsoncxx::document::view_or_value outputProjection = bsoncxx::from_json(fieldProjection.c_str());
    auto resultFormat = opts.projection(outputProjection);
    mongocxx::v_noabi::cursor cursor = collection.find(filter.view(), resultFormat);
    bsoncxx::document::view res = *cursor.begin();
    mongocxx::cursor::iterator iter = cursor.begin();

    if(iter == cursor.end()) {
        return(std::string());
    }

    std::stringstream result("");
    result << "[";

    for (auto&& doc : cursor) {
        //ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [worker:%t] %M %N:%l Shipment List %s\n"), bsoncxx::to_json(doc).c_str()));
        result << bsoncxx::to_json(doc).c_str()
               << ",";
    }
    result.seekp(-1, std::ios_base::end);
    result << "]";

    return(std::string(result.str()));

    #if 0
    //bsoncxx::stdx::optional<mongocxx::result::update> result = collection.update_many(filter.view(), toUpdate.view());
    bsoncxx::stdx::optional<mongocxx::cursor> result = collection.find(filter.view());
    bsoncxx::document::value what = bsoncxx::from_json(key.c_str());
    json_object.clear();
    bsoncxx::stdx::optional<bsoncxx::document::value> result = get_collection(CollectionName::SHIPPING).find_one(what);
    json_object =  bsoncxx::to_json(result.view());
    #endif
}

std::int32_t Mongodbc::create_bulk_shipment(std::string bulkShipment)
{
    mongocxx::options::bulk_write bulk_opt;
    mongocxx::write_concern wc;
    bulk_opt.ordered(false);
    wc.acknowledge_level(mongocxx::write_concern::level::k_default);
    bulk_opt.write_concern(wc);

    bsoncxx::document::value new_shipment = bsoncxx::from_json(bulkShipment.c_str());
    auto conn = mMongoConnPool->acquire();
    mongocxx::database dbInst = conn->database(get_dbName().c_str());
    auto collection = dbInst.collection("shipping");

    auto bulk = collection.create_bulk_write(bulk_opt);

    bsoncxx::document::view dock_view = new_shipment.view();
    auto iter = dock_view.begin();
    
    //bsoncxx::document::element uid = dock_view["0"];
    //ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [worker:%t] %M %N:%l Bulk-Shipment\n")));
    //ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [worker:%t] %M %N:%l Bulk-Shipment %s\n"), bsoncxx::to_json(uid.get_document().value).c_str()));

    for(; iter != dock_view.end(); ++iter) {
        bsoncxx::document::element elm = *iter;
        //bsoncxx::document::view elm_view = elm.get_document().value;
        mongocxx::model::insert_one insert_op(elm.get_document().value);
        bulk.append(insert_op);
    }

    auto result = bulk.execute();
    std::int32_t cnt = 0;

    if(result) {
        cnt = result->inserted_count();
        ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [worker:%t] %M %N:%l bulk document created is %d\n"), cnt));
    }

    return(cnt);
}

/*
std::int32_t Mongodbc::update_bulk_shipment(std::string bulkShipment)
{

}
*/
#endif /* __mongodbc_cc__*/
