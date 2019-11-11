#include <QString>
#include <map>

namespace Common {
enum class ChunkType { Normal, Highlighted, Selected, Info, Warning, Error, Quote, Date, Component };
static const std::map<Common::ChunkType, QString> preDefineList = { 
	{ ChunkType::Info, QStringLiteral( "\\binfo\\b" ) },
    { ChunkType::Warning, QStringLiteral( "\\bwarning\\b" ) },
    { ChunkType::Error, QStringLiteral( "\\berror\\b" ) },
    { ChunkType::Quote, QStringLiteral( "'.*'" ) },
    { ChunkType::Date, QStringLiteral( "[12]\\d{3}\\.(0[1-9]|1[0-2])\\.(0[1-9]|[12]\\d|3[01])" ) },
    { ChunkType::Component, QStringLiteral( "\\(.*?\\)" ) },
 };
} // namespace Common
