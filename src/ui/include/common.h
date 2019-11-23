#include <QString>
#include <map>

namespace Common {
enum class ChunkType { Normal, Highlighted, Selected, Info, Warning, Error, Quote, Common, Component, Number };
static const std::multimap<Common::ChunkType, QString> preDefineList = { 
	{ ChunkType::Info, QStringLiteral( "\\binfo\\b" ) },
    { ChunkType::Info, QStringLiteral( "\\bsuccessfully\\b" ) },
    { ChunkType::Info, QStringLiteral( "\\bok\\b" ) },
    { ChunkType::Warning, QStringLiteral( "\\bwarning\\b" ) },
    { ChunkType::Warning, QStringLiteral( "\\btimeout\\b" ) },
    { ChunkType::Warning, QStringLiteral( "\\btimed out\\b" ) },
    { ChunkType::Warning, QStringLiteral( "\\babort\\b" ) },
    { ChunkType::Error, QStringLiteral( "\\berror\\b" ) },
    { ChunkType::Error, QStringLiteral( "\\bfailed\\b" ) },
    { ChunkType::Error, QStringLiteral( "\\bfail\\b" ) },
    { ChunkType::Error, QStringLiteral( "\\bcrashed\\b" ) },
    { ChunkType::Error, QStringLiteral( "\\bcrash\\b" ) },
    //{ ChunkType::Quote, QStringLiteral( "(\"([^\"]|\"\")*\")|('([^']|'')*')" ) },
    { ChunkType::Common, QStringLiteral( "[12]\\d{3}\\.(0[1-9]|1[0-2])\\.(0[1-9]|[12]\\d|3[01])" ) },
    { ChunkType::Common, QStringLiteral( "\\bmsgid\\b" ) },
    { ChunkType::Number, QStringLiteral( "[A-Fa-f0-9]{64}" ) },  //Hash SHA256
    //{ ChunkType::Number, QStringLiteral( "\\d+" ) },
    //{ ChunkType::Component, QStringLiteral( "\\(.*?\\)" ) },
 };
} // namespace Common
