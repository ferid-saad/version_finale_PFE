package com.iset.saa.models;

public class SystemData {
    private boolean motopompeStatus, vane1Status, vane2Status;
    private float temp, humSol, hum;

    public SystemData() {
        this.motopompeStatus = false;
        this.vane1Status = false;
        this.vane2Status = false;
        this.temp = 0;
        this.hum = 0;
        this.humSol = 0;
    }

    public SystemData(
            boolean motopompeStatus,
            boolean vane1Status,
            boolean vane2Status,
            float temp,
            float humSol,
            float hum
    ) {

        this.motopompeStatus = motopompeStatus;

        this.vane1Status = vane1Status;
        this.vane2Status = vane2Status;
        this.temp = temp;
        this.hum = hum;
        this.humSol = humSol;
    }

    public boolean isMotopompeStatus() {
        return motopompeStatus;
    }

    public void setMotopompeStatus(boolean motopompeStatus) {
        this.motopompeStatus = motopompeStatus;
    }

    public boolean isVane1Status() {
        return vane1Status;
    }

    public void setVane1Status(boolean vane1Status) {
        this.vane1Status = vane1Status;
    }

    public boolean isVane2Status() {
        return vane2Status;
    }

    public void setVane2Status(boolean vane2Status) {
        this.vane2Status = vane2Status;
    }

    public float getTemp() {
        return temp;
    }

    public void setTemp(float temp) {
        this.temp = temp;
    }

    public float getHumSol() {
        return humSol;
    }

    public void setHumSol(float humSol) {
        this.humSol = humSol;
    }

    public float getHum() {
        return hum;
    }

    public void setHum(float hum) {
        this.hum = hum;
    }
}
