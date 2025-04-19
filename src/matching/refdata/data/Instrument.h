#include <string>

class Instrument
{
private:
    int m_instrumentID;
    std::string m_instrumentName;
public:
    Instrument(int id, const std::string& instrumentName) :
        m_instrumentID(id),
        m_instrumentName(std::move(instrumentName)){};
    Instrument() : Instrument(-1, ""){};
    
    
    const auto getInstrumentID() const { return m_instrumentID; }
    const auto getInstrumentName() const { return m_instrumentName; }
    
    auto toString() {return std::move(std::to_string(m_instrumentID) + "-" + m_instrumentName); }
};
