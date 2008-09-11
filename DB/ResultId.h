#ifndef DB_RESULTID_H
#define DB_RESULTID_H

#include <QString>
#include <KSharedPtr>
#include "Result.h"
#include "ImageInfo.h"

namespace DB
{
    // TODO: to be renamed to Id as per discussion in car.
class ResultId {
public:
    static const ResultId null;

    /* The default constructor creates a 'null' instance of ResultId.
     * Consider using DB::ResultId::null instance instead
     */
    ResultId();

    /** Construct with given fileId and context.
     *
     * \pre !context.isNull()
     */
    ResultId(int fileId, const ConstResultPtr& context);

    static ResultId createContextless(int fileId)
    {
        return ResultId(fileId);
    }

    int fileId() const;
    bool isNull() const;

    /** Get context of this.
     *
     * \note result might be null
     */
    ConstResultPtr context() const;

    inline bool operator==(const ResultId& other) const {
        return other._fileId == _fileId;  // we're only interested in the id.
    }
    inline bool operator!=(const ResultId& other) const {
        return other._fileId != _fileId;  // we're only interested in the id.
    }
    inline bool operator<(const ResultId& other) const {
        return _fileId < other._fileId;
    }

    /** Convenience method: fetch the associated ImageInfo for this ID */
    ImageInfoPtr fetchInfo() const;

 private:
    ResultId(int fileId)
        : _fileId(fileId)
        , _context(0)
    {
        Q_ASSERT(fileId >= 0);
    }

    int _fileId;
    ConstResultPtr _context;
};
}  // namespace DB

/*
 * qHash() so that we can put the ResultId in a QSet<> or QHash<> as key.
 *
 * The Qt documentation talks about putting this in the global namespace. But
 * that does not work.
 *
 * Due to some name lookup wierdness, it only works if we enforce this method
 * to be declared before including <QHash> (hard to do, because QHash might be
 * implicitly included in any header before, i.e. that means that ResultId.h
 * would need to be included always as first header), or just put it in the
 * DB namespace. Either this is a problem in gcc or in Qt (forgot to explicitly
 * address the global namesapce with ::qHash() ?).
 * TODO(hzeller): figure out a better solution.
 * ( http://gcc.gnu.org/bugzilla/show_bug.cgi?id=26311 )
 */
namespace DB {
    inline unsigned int qHash(const DB::ResultId& id) { return id.fileId(); }
}  // namespace DB

#endif /* DB_RESULTID_H */

