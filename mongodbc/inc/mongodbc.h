#ifndef __mongodbc_h__
#define __mongodbc_h__

#include <cstdint>
#include <iostream>
#include <vector>
#include <mutex>

#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/stdx.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/instance.hpp>
#include <bsoncxx/builder/stream/helpers.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/array.hpp>


using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::open_document;

enum class CollectionName : std::uint32_t {
    SHIPPING = 0,
    JOB_SCHEDULING,
    MANIFESTING,
    SUPPLIES,
    SHIPMENT_PRICING,

    TRACKING,
    REPORTING,

    INVALID
};

class Mongodbc {
    public:
        Mongodbc(std::string uri, std::string db_name);
        Mongodbc();
        ~Mongodbc();

        std::string get_dbName() const {
            return(mdbName);
        }

        void set_dbName(std::string dbName) {
            mdbName = dbName;
        }

        std::string get_uri() const {
            return(mURI);
        }

        void set_uri(std::string uri) {
            mURI = uri;
        }

        mongocxx::database& get_mongoDbInst() {
            return(mMongoDbInst);
        }

        mongocxx::collection& get_collection(CollectionName tableName) {
            return(mMongoCollections[static_cast<size_t>(tableName)]);
        }

        bool create_shipment(std::string shippingRecord);
        bool update_shipment(std::string shippingRecord);
        bool delete_shipment(std::string shippingRecord);
        void dump_document(bsoncxx::document::value& doc, CollectionName collection);

    private:
        std::string mURI;
        std::string mdbName;
        std::array<std::string, 32>mCollections;
        mongocxx::database mMongoDbInst;
        std::array<mongocxx::collection, 64> mMongoCollections;
        mongocxx::client* mMongoConn;
        mongocxx::instance* mInstance;
        mongocxx::uri mMongoUri;
        std::mutex mMutex;
};


#endif /*__mongodbc_h__*/