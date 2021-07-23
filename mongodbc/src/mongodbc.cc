#ifndef __mongodbc_cc__
#define __mongodbc_cc__

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
            poolUri += "/?minPoolSize=10&maxPoolSize=" + std::to_string(poolSize);
            mongocxx::uri uri(poolUri.c_str());
            mMongoConnPool = new mongocxx::pool(uri);

            if(nullptr == mMongoConnPool) {
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

bool Mongodbc::create_shipment(std::string shipmentRecord)
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
    }

    return(true);
}

bool Mongodbc::update_shipment(std::string match, std::string shippingRecord)
{
    #if 0
    bsoncxx::document::value new_shipment = bsoncxx::from_json(shippingRecord.c_str());
    bsoncxx::document::value match = bsoncxx::from_json(match.c_str());
    mMutex.lock();
    bsoncxx::stdx::optional<mongocxx::result::update> result = get_collection(CollectionName::SHIPPING).update_one(match, new_shipment.view());
    mMutex.unlock();
    #endif
    return(true);

}

bool Mongodbc::delete_shipment(std::string shippingRecord)
{
    return(true);
}
std::string Mongodbc::get_shipment(std::string key)
{
    std::string json_object;
    #if 0
    bsoncxx::document::value what = bsoncxx::from_json(key.c_str());
    json_object.clear();
    bsoncxx::stdx::optional<bsoncxx::document::value> result = get_collection(CollectionName::SHIPPING).find_one(what);
    json_object =  bsoncxx::to_json(result.view());
    #endif
    return(json_object);
}
#endif /* __mongodbc_cc__*/