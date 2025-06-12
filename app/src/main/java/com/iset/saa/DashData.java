package com.iset.saa;

import com.google.firebase.database.Exclude;
import com.google.firebase.database.IgnoreExtraProperties;

import java.util.Map;
import java.util.HashMap;

@IgnoreExtraProperties
public class DashData {
    public Long temperature;
    public Long temperatureExt;
    public Long humidite;
    public Long humiditeSol;
    public Long humiditeExt;
    public Long vitesseVent;

    public Boolean motopompe;
    public Boolean vanne1;
    public Boolean vanne2;

    public DashData() {

    }
    public DashData(
            Long temperature,
            Long humidite,
            Long humiditeSol,
            boolean motopompe,
            Long temperatureExt,
            Long humiditeExt,
            Long vitesseVent,
            boolean vanne1,
            boolean vanne2
    ) {
        this.temperature = temperature;
        this.humidite = humidite;
        this.humiditeSol = humiditeSol;
        this.temperatureExt = temperatureExt;
        this.humiditeExt = humiditeExt;
        this.vitesseVent = vitesseVent;
        this.motopompe = motopompe;
        this.vanne1 = vanne1;
        this.vanne2 = vanne2;
    }

    @Exclude
    public Map<String, Object> toMap() {
        HashMap<String, Object> result = new HashMap<>();
        result.put("temperature", temperature);
        result.put("humidite", humidite);
        result.put("temperatureExt", temperatureExt);
        result.put("humiditeExt", humiditeExt);
        result.put("windExt", vitesseVent);
        result.put("humiditeSol", humiditeSol);
        result.put("motopompe", motopompe);
        result.put("vanne1", vanne1);
        result.put("vanne2", vanne2);
        return result;
    }
}
