import numpy as np
import pandas as pd
import datetime as dt

import matplotlib.dates as mdates
import matplotlib.pyplot as plt
import seaborn as sns

class Analyzer:

    def __init__(self, data, target_ch=0):

        self.data = data
        self.target_ch = target_ch
        self.pmax = data.iloc[:, target_ch].max()

        self.power = self.convert_df(data)
        self.ndays = self.power.shape[0]

        self.dates = pd.date_range(start=self.data.index[0], end=self.data.index[-1])


    def convert_df(self, data, ch=0):

        t = data.index
        x = data.iloc[:, ch]

        d = [x.day for x in t]
        h = [x.hour for x in t]

        dt = np.diff(t.to_numpy().astype(np.float64))
        p = x.to_numpy()[:-1] * dt / (1e+9 * 60 * 60)

        df = pd.DataFrame({'day':d[:-1], 'hour': h[:-1], 'power':p})
        power = df.groupby(['day', 'hour']).sum().unstack()['power']
    
        return power


    def plot(self, max_power=300):

        t = self.data.index
        x = self.data.iloc[:, self.target_ch].to_numpy()

        _, axes = plt.subplots(self.ndays, 1, figsize=(10, 2.3 * self.ndays))

        for k, d in enumerate(self.dates):

            idx = (t.day == d.day) & (t.month == d.month) & (t.year == d.year)

            xmin = dt.datetime(d.year, d.month, d.day, 0, 0) - dt.timedelta(hours=1)
            xmax = xmin + dt.timedelta(hours=26)

            axes[k].plot(t[idx], x[idx])
            axes[k].set_xlim([xmin, xmax])
            axes[k].set_ylim([0, self.pmax * 1.1])
            axes[k].set_ylabel('Power [VA]')
            axes[k].xaxis.set_major_formatter(mdates.DateFormatter("%H:%M"))

        plt.show()


    def plot_heatmap(self):

        plt.figure(figsize=(12, 0.4 * self.ndays))
        sns.heatmap(self.power, cmap='jet')
        plt.show()


    def plot_bar(self):

        plt.figure(figsize=(self.ndays, 3))

        power_daily = self.power.iloc[1:-1, :].sum(axis=1)
    
        sns.barplot(x=power_daily.index, y=power_daily.values, color='blue')
        plt.show()


    def estimate_monthly_cost(self, kwh_price=25):

        power_daily = self.power.iloc[1:-1, :].sum(axis=1)
        estimated_monthly_cost = power_daily.mean() * kwh_price * 30 / 1e+3

        return estimated_monthly_cost
