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

Mongodbc::Mongodbc(std::string uri_str, std::string db_name)
{
    mMongoConn = nullptr;
    mInstance = nullptr;
    mURI = uri_str;
    mdbName = db_name;

    mInstance = new mongocxx::instance();
    do {
        if(nullptr == mInstance) {
            break;
        }

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

void Mongodbc::dump_document(bsoncxx::document::value& doc, CollectionName collection)
{

    auto cursor = get_collection(collection).find({});
 
    for (auto&& doc : cursor) {
        std::cout << bsoncxx::to_json(doc) << std::endl;
    }

}

bool Mongodbc::create_shipment(std::string shipmentRecord)
{
    bsoncxx::document::value new_shipment = bsoncxx::from_json(shipmentRecord.c_str());
    //bsoncxx::stdx::optional<mongocxx::result::insert_one> result = mMongoCollections[static_cast<size_t>(CollectionName::SHIPPING)].insert_one(new_shipment.view());
    mMutex.lock();
    bsoncxx::stdx::optional<mongocxx::result::insert_one> result = get_collection(CollectionName::SHIPPING).insert_one(new_shipment.view());
    mMutex.unlock();

}

#endif /* __mongodbc_cc__*/