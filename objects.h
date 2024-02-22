#pragma once
#include <Arduino.h>
#include <DallasTemperature.h>
#define NOT_IMPLEMENETED "Not implemented"

String str(int val) { return (String)std::to_string(val).c_str(); }

class AElement {
    public:
    virtual float get_value() = 0;
    virtual void set_value(bool value) = 0;
    virtual String get_name() = 0;
    virtual String print_HTML() = 0;
};

class HTML : public AElement {
    public:
    HTML() { }
    virtual float get_value() override { throw std::runtime_error(NOT_IMPLEMENETED); }
    virtual void set_value(bool value) override { throw std::runtime_error(NOT_IMPLEMENETED); } 
    virtual String get_name() override { return "HTML"; }
    virtual String print_HTML() override {
        return R"(<!DOCTYPE html>
        <html lang="en" data-bs-theme="dark">
            <head>
                <meta charset="utf-8">
                <meta name="viewport" content="width=device-width, initial-scale=1">
                <title>ESP32 WROOM</title>
                <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.2/dist/css/bootstrap.min.css" rel="stylesheet" integrity="sha384-T3c6CoIi6uLrA9TneNEoa7RxnatzjcDSCmG1MXxSR1GAsXEV/Dwwykc2MPK8M2HN" crossorigin="anonymous">
            </head>
            <body>
                <script src="https://cdn.jsdelivr.net/npm/bootstrap@5.3.2/dist/js/bootstrap.bundle.min.js" integrity="sha384-C6RzsynM9kWDrMNeT87bh95OGNyZPhcTNXj1NW7RuBCsyN/o0jlpcV8Qyq46cDfL" crossorigin="anonymous"></script>
                <script>
                    var websocket, toSend = {};
                    window.addEventListener("load", onLoad);

                    function sendData() {
                        let data = JSON.stringify(toSend);
                        websocket.send(data);
                    }

                    function onClose(event) { setTimeout(onLoad, 2000); }

                    function onLoad() {
                        websocket = new WebSocket(`ws://${window.location.hostname}/ws`);
                        websocket.onclose = onClose;
                        websocket.onmessage = onMessage;
                    }

                    function onMessage(event) {
                        let json = JSON.parse(event.data);
                        for (let element in json) document.getElementById(element).innerText = json[element];
                    }
                </script>
            </body>
        </html>)";
    }
};

class Diode : public AElement {
    ushort diode_pin;
    public:
    Diode(ushort pin) {
        diode_pin = pin;
        pinMode(diode_pin, OUTPUT);
    }
    virtual float get_value() override { return digitalRead(diode_pin) * 100; }
    virtual void set_value(bool value) override { digitalWrite(diode_pin, value); }
    virtual String get_name() override { return "led_" + str(diode_pin); }
    virtual String print_HTML() override {
        String initial = R"(<body>
            <div class="container mt-5">
                <div class="row justify-content-center">
                    <div class="col-md-6">
                        <div class="card text-center">
                            <div class="card-header">LED on DIODE_PIN</div>
                            <div class="card-body">
                                <h5 class="card-title">Luminance (%)</h5>
                                <h2 id="led_DIODE_PIN"></h2>
                            </div>
                            <input type="checkbox" class="btn-check" id="led_btn_DIODE_PIN" autocomplete="off">
                            <label class="btn btn-outline-success" for="led_btn_DIODE_PIN">LED</label>
                        </div>
                    </div>
                </div>
            </div>
            <script>
                document.getElementById("led_btn_DIODE_PIN").addEventListener("click", led_btn_click);
                function led_btn_click() {
                    let name = this.id.replace("btn_", "");
                    toSend[name] = this.checked; sendData();
                }
            </script>
        </body>)";
        initial.replace("DIODE_PIN", str(diode_pin));
        return initial;
    }
};

class Photoresistor : public AElement {
    ushort max_value, photoresistor_pin;
    public:
    Photoresistor(ushort pin, ushort max) {
        photoresistor_pin = pin;
        max_value = max;
    }
    virtual float get_value() override { return ((max_value - analogRead(photoresistor_pin)) * 100) / max_value; }
    virtual void set_value(bool value) override { throw std::runtime_error(NOT_IMPLEMENETED); } 
    virtual String get_name() override { return "photoresistor_" + str(photoresistor_pin); }
    virtual String print_HTML() override {
        String initial =  R"(<body>
            <div class="container mt-5">
                <div class="row justify-content-center">
                    <div class="col-md-6">
                        <div class="card text-center">
                            <div class="card-header">Photoresistor on PHOTORESISTOR_PIN</div>
                            <div class="card-body">
                                <h5 class="card-title">Luminance (%)</h5>
                                <h2 id="photoresistor_PHOTORESISTOR_PIN"></h2>
                            </div>
                        </div>
                    </div>
                </div>
            </div>
        </body>)";
        initial.replace("PHOTORESISTOR_PIN", str(photoresistor_pin));
        return initial;
    }
};

class DS18B20 : public AElement {
    byte index;
    DallasTemperature* sensors;
    public:
    DS18B20(DallasTemperature* dt, byte i) {
        sensors = dt;
        index = i;
    }
    virtual float get_value() override {
        sensors->requestTemperatures(); 
        return sensors->getTempCByIndex(index);
    }
    virtual void set_value(bool value) override { throw new std::runtime_error(NOT_IMPLEMENETED); } 
    virtual String get_name() override { return "temperature_" + str(index); }
    virtual String print_HTML() override {
        String initial = R"(<body>
            <div class="container mt-5">
                <div class="row justify-content-center">
                    <div class="col-md-6">
                        <div class="card text-center">
                            <div class="card-header">DS18B20 #INDEX</div>
                            <div class="card-body">
                                <h5 class="card-title">Temperature (°C)</h5>
                                <h2 id="temperature_INDEX"></h2>
                            </div>
                        </div>
                    </div>
                </div>
            </div>
        </body>)";
        initial.replace("INDEX", str(index));
        return initial;
    }
};