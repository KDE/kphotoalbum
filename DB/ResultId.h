#ifndef RESULTID_H
#define RESULTID_H

#include <QString>

namespace XMLDB { class Database;}
namespace SQLDB { class Database;}

namespace DB
{
class Result;


class ResultId
{
public:
    ResultId(int fileId, const Result& context); // QUERTY rewrite to shared pointer.
    int fileId() const;

private:
    int _fileId;
    const DB::Result& _context;
};

};


#endif /* RESULTID_H */

