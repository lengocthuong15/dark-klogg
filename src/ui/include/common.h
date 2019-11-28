#include <QString>
#include <map>

namespace Common {
enum class ChunkType { Normal, Highlighted, Selected, Info, Warning, Error, Quote, Common, Component, Number, MD5 };
static const std::multimap<Common::ChunkType, QString> preDefineList = { 
	{ ChunkType::Info, QStringLiteral( "\\binfo\\b" ) },
    { ChunkType::Info, QStringLiteral( "\\bsuccessfully\\b" ) },
	{ ChunkType::Info, QStringLiteral("\\bok\\b") },
	{ ChunkType::Info, QStringLiteral( "\\bstart\\b" ) },
    { ChunkType::Warning, QStringLiteral( "\\bwarning\\b" ) },
    { ChunkType::Warning, QStringLiteral( "\\btimeout\\b" ) },
	{ ChunkType::Warning, QStringLiteral("\\btimed out\\b") },
	{ ChunkType::Warning, QStringLiteral("\\bterminate\\b") },
	{ ChunkType::Warning, QStringLiteral( "\\bterminated\\b" ) },
	{ ChunkType::Warning, QStringLiteral("\\babort\\b") },
	{ ChunkType::Warning, QStringLiteral( "\\bend\\b" ) },
    { ChunkType::Error, QStringLiteral( "\\berror\\b" ) },
    { ChunkType::Error, QStringLiteral( "\\bfailed\\b" ) },
    { ChunkType::Error, QStringLiteral( "\\bfail\\b" ) },
    { ChunkType::Error, QStringLiteral( "\\bcrashed\\b" ) },
    { ChunkType::Error, QStringLiteral( "\\bcrash\\b" ) },
    //{ ChunkType::Quote, QStringLiteral( "(\"([^\"]|\"\")*\")|('([^']|'')*')" ) },
    { ChunkType::Component, QStringLiteral( "[12]\\d{3}\\.(0[1-9]|1[0-2])\\.(0[1-9]|[12]\\d|3[01])" ) },
    { ChunkType::Component, QStringLiteral( "\\bmsgid\\b" ) },
	{ ChunkType::Number, QStringLiteral("[A-Fa-f0-9]{64}") },  //Hash SHA256
	//{ ChunkType::MD5, QStringLiteral( "[A-Fa-f0-9]{32}" ) },  //Hash MD5
    //{ ChunkType::Number, QStringLiteral( "\\d+" ) },
    //{ ChunkType::Component, QStringLiteral( "\\(.*?\\)" ) },
 };
} // namespace Common
