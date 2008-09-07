#ifndef RESULTID_H
#define RESULTID_H

#include <QString>
#include <KSharedPtr>
#include "Result.h"

namespace XMLDB { class Database;}
namespace SQLDB { class Database;}

namespace DB
{
class ResultId {   // to be renamed just 'Id' as per discussion in car.
public:
    static const ResultId null;

    /* The default constructor creates a 'null' instance of ResultId.
     * Consider using DB::ResultId::null instance instead
     */
    ResultId();
    ResultId(int fileId, const ConstResultPtr& context);

    int fileId() const;
    bool isNull() const;
    bool operator==(const ResultId& other) const;

 private:
    int _fileId;
    ConstResultPtr _context;
};
}  // namespace DB

/** qHash() so that we can put the ResultId in a QSet<>*/
inline uint qHash(const DB::ResultId &id) { return id.fileId(); }


#endif /* RESULTID_H */

