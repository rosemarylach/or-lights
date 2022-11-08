from kivy.app import App
from kivy.uix.floatlayout import FloatLayout
from kivy.uix.boxlayout import BoxLayout
import time

# root widget for .kv file
class ZoilDefWidget(FloatLayout):
    def capture(self):
        '''
        Function to capture the images and give them the names
        according to their captured time and date.
        '''
        camera = self.ids['camera']
        timestr = time.strftime("%Y%m%d_%H%M%S")
        camera.export_to_png("IMG_{}.png".format(timestr))
        print("Captured")

class ZoilDefApp(App):
    title = "ZoilDef App"

    def build(self):
        widget =  ZoilDefWidget()
        return widget

    def on_pause(self):
        return True

if __name__ == '__main__':
    ZoilDefApp().run()