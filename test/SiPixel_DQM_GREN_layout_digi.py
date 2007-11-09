def pixellayout(i, p, *rows): i["Layouts/SiPixel Digi Layouts/" + p] = DQMItem(layout=rows)

pixellayout(dqmitems, "SiPixel_Digi_Summary",
  ["Tracker/SUMDIG_adc_PixelBarrel",
   "Tracker/SUMDIG_ndigis_PixelBarrel"],
  ["Tracker/SUMDIG_adc_PixelEndcap",
   "Tracker/SUMDIG_ndigis_PixelEndcap"])
