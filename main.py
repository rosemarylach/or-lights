from kivy.app import App
from kivy.uix.floatlayout import FloatLayout

# root widget for .kv file
class ZoilDefWidget(FloatLayout):
    pass

class ZoilDefApp(App):
    title = "ZoilDef App"

    def build(self):
        widget =  ZoilDefWidget()
        return widget

    def on_pause(self):
        return True

if __name__ == '__main__':
    ZoilDefApp().run()