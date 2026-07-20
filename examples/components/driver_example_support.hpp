#pragma once
#include <cstddef>
#include <cstdint>
#include <eurorack/io/analog_io.hpp>
#include <eurorack/io/digital_io.hpp>
#include <eurorack/io/i2c_bus.hpp>
#include <eurorack/io/io_result.hpp>
#include <eurorack/io/spi_bus.hpp>
#include <eurorack/io/time_source.hpp>

namespace example {
class DigitalOutput final : public eurorack::io::DigitalOutput {
 public:
  void writeHigh(bool high) noexcept override { high_ = high; ++writes_; }
  [[nodiscard]] bool lastWrittenHigh() const noexcept override { return high_; }
  [[nodiscard]] std::size_t writes() const noexcept { return writes_; }
 private: bool high_{false}; std::size_t writes_{0U};
};
class SpiBus final : public eurorack::io::SpiBus {
 public:
  eurorack::io::IoResult beginTransaction(const eurorack::io::SpiSettings& settings) noexcept override { settings_=settings; active_=true; return eurorack::io::IoResult::Success; }
  eurorack::io::IoResult transfer(const std::uint8_t* tx,std::uint8_t* rx,std::size_t size) noexcept override {
    if(!active_) return eurorack::io::IoResult::Busy;
    size_=size; for(std::size_t i=0;i<size;++i){ const auto value=tx?tx[i]:std::uint8_t{0U}; if(i<128U) bytes_[i]=value; if(rx) rx[i]=receiveValue_; }
    ++transfers_; return eurorack::io::IoResult::Success;
  }
  void endTransaction() noexcept override { active_=false; }
  void setReceiveValue(std::uint8_t value) noexcept { receiveValue_=value; }
  [[nodiscard]] std::size_t transfers() const noexcept { return transfers_; }
  [[nodiscard]] std::size_t size() const noexcept { return size_; }
 private:
  bool active_{false}; std::uint8_t receiveValue_{0U}; std::size_t transfers_{0U}; std::size_t size_{0U}; eurorack::io::SpiSettings settings_{}; std::uint8_t bytes_[128U]{};
};
class I2cBus final : public eurorack::io::I2cBus {
 public:
  eurorack::io::IoResult setClock(std::uint32_t hz) noexcept override { clock_=hz; return hz?eurorack::io::IoResult::Success:eurorack::io::IoResult::InvalidArgument; }
  eurorack::io::IoResult write(eurorack::io::I2cAddress address,const std::uint8_t* data,std::size_t size,bool) noexcept override { if(!address.isValid()||(size&&data==nullptr)) return eurorack::io::IoResult::InvalidArgument; ++writes_; bytes_+=size; return eurorack::io::IoResult::Success; }
  eurorack::io::IoResult read(eurorack::io::I2cAddress address,std::uint8_t* data,std::size_t size) noexcept override { if(!address.isValid()||(size&&data==nullptr)) return eurorack::io::IoResult::InvalidArgument; for(std::size_t i=0;i<size;++i) data[i]=0U; return eurorack::io::IoResult::Success; }
  eurorack::io::IoResult writeRead(eurorack::io::I2cAddress address,const std::uint8_t* wd,std::size_t ws,std::uint8_t* rd,std::size_t rs) noexcept override { auto r=write(address,wd,ws,false); return r==eurorack::io::IoResult::Success?read(address,rd,rs):r; }
  [[nodiscard]] std::size_t writes() const noexcept { return writes_; }
 private: std::uint32_t clock_{0U}; std::size_t writes_{0U}; std::size_t bytes_{0U};
};
class AnalogInput final : public eurorack::io::AnalogInputChannel {
 public:
  explicit AnalogInput(std::uint32_t code=2048U) noexcept:code_(code){}
  [[nodiscard]] eurorack::io::AnalogSample readRaw() noexcept override { return {code_,eurorack::io::IoResult::Success}; }
  [[nodiscard]] std::uint32_t maximumCode() const noexcept override { return 4095U; }
 private: std::uint32_t code_;
};
class Delay final : public eurorack::io::DelayProvider { public: void delayMicroseconds(std::uint32_t us) noexcept override { elapsed_+=us; } [[nodiscard]] std::uint32_t elapsed() const noexcept{return elapsed_;} private:std::uint32_t elapsed_{0U};};
}
