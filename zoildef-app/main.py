from kivymd.app import MDApp
from kivy.lang import Builder
from kivy.core.window import Window
from kivy.app import App
from kivy.graphics import Color

from kivy.graphics import Line
from kivy.uix.widget import Widget

from kivy.uix.screenmanager import ScreenManager, Screen
import time
import socket

from kivy.uix.image import Image
from kivy.clock import Clock
from kivy.graphics.texture import Texture
import cv2

import math
import sympy
from sympy import symbols, solve, lambdify
from scipy.optimize import fsolve

class Config():

    def __init__(self):
        # ZOIL Selection
        self.x = 0
        self.y = 0
        self.h = 2.5
        self.b = 50

        # CONSTANTS FOR CALCULATIONS ----------------
        self.X_OFFSET = 36 #in, offset from (0, 0) to center of panel
        self.Y_OFFSET = 36 #in, offset from (0, 0) to center of panel
        self.H_A = 1.76 #in, offset from 80-20 structure to pitch axis
        self.P = 0.46 + 7.56 #in, distance from center of pitch axis to face of side lights

        # useful dimensions of the light w/ respect to rotation points
        self.B_1 = 0.89 #in
        self.B_2 = 1.6 #in
        self.B_3 = 1.29 #in
        self.B_4 = 1.83 #in

        self.R_AXIS = 2 #in, distance from light rotation point to the center axis of focus mechanism

        ## NOT CORRECT, NEEDS TO BE UPDATED
        self.L_ARM = 3 #in, length of linkage (between the two rotation points)
        self.R_TOP = 1.83 #in, length between top linkage rotation point and center axis of focus mechanism

        self.KAPPA_0 = 3.42 #in, distance from light rotation point to bottom face of hexagon plate when focus mechanism is at d = 0

        self.LIGHT_DIAMETER = 1.96 #in
        self.LIGHT_ANGLE = 5 #degrees
        
        # END CALC CONSTANTS ------------------------

        # Network Information
        self.cam_ip = "168.5.73.91"
        self.my_ip = "168.5.90.169"
        self.teaching_ips = ["", ""]
        self.port = 50000
        self.timeout = 20.0 # timeout to accept connection from arduino
        # self.conn, self.conn_addr = self.connect()

    def calc_pitch(self):
        # distance from panel's center to center of ZOIL, projected onto lit surface, in
        l_1 = math.sqrt((self.y_offset + self.y) ** 2 + (self.x_offset + self.x) ** 2)
        l_2 = math.sqrt((self.y_offset + self.y) ** 2 + (self.x_offset - self.x) ** 2)
        l_3 = math.sqrt((self.y_offset - self.y) ** 2 + (self.x_offset + self.x) ** 2)
        l_4 = math.sqrt((self.y_offset - self.y) ** 2 + (self.x_offset - self.x) ** 2)

        # distance from lit surface to pitch axis center, in
        h_c = self.h - self.H_A

        # pitch calculations, degrees
        gamma_1 = - math.degrees(math.atan(h_c / l_1))
        gamma_2 = math.degrees(math.atan(h_c / l_2))
        gamma_3 = math.degrees(math.atan(h_c / l_3))
        gamma_4 = -math.degrees(math.atan(h_c / l_4))

        return [gamma_1, gamma_2, gamma_3, gamma_4], [l_1, l_2, l_3, l_4]

    def calc_yaw(self):
        theta_1 = - math.degrees(math.atan((self.y_offset + self.y) / (self.x_offset + self.x)))
        theta_2 = math.degrees(math.atan((self.y_offset + self.y) / (self.x_offset - self.x)))
        theta_3 = math.degrees(math.atan((self.y_offset - self.y) / (self.x_offset + self.x)))
        theta_4 = math.degrees(math.atan((self.y_offset - self.y) / (self.x_offset - self.x)))

        return [theta_1, theta_2, theta_3, theta_4]
    
    def calc_focus(self, pitch, l_dists):
        # declare symbolic variables for alphas
        alpha_syms = [ symbols('alpha_1_sym'),
                       symbols('alpha_2_sym'),
                       symbols('alpha_3_sym'),
                       symbols('alpha_4_sym') ]

        # offset from pitch axis to side light height, in
        h_bs = [self.P * math.cos(math.radians(gamma)) for gamma in pitch]

        # average distance that light will travel to reach lit surface, in
        lambdas = [math.sqrt(l ** 2 + (self.h - self.H_A - h_b) ** 2) for l, h_b in zip(l_dists, h_bs)]

        # diameter of projected light from side lights, in
        dsls = [self.LIGHT_DIAMETER + 2 * lam * math.tan(math.radians(self.LIGHT_ANGLE)) for lam in lambdas]

        # distance from center of ZOIL to edge of projected light by side light (circumradius of "hexagon"), in
        a = 2 / math.sqrt(3) * self.r

        # distance from center of ZOIL to center of projected light by side light, in
        qs = [a - 1/2 * dsl for dsl in dsls]

        # change in distance from light center to panel focus mechanism center due to tilt of light
        # SYMBOLIC
        delta_rs = [self.B_1 * sympy.cos(alpha) - self.B_2 * sympy.sin(alpha) for alpha in alpha_syms]

        # distance between center of light and panel focus mechanism center
        # SYMBOLIC
        r_bcs = [self.R_AXIS + delta_r for delta_r in delta_rs]

        # distance from center of projected light by side light to center of side light projected downwards onto lit surface, in
        # SYMBOLIC
        ms = [r_bc - q for r_bc, q in zip(r_bcs, qs)]

        # expression for each of the alphas
        eqs = [sympy.atan(m / lam) - alpha for m, lam, alpha in zip(ms, lambdas, alpha_syms)]

        # convert to numpy for solving numerically instead of analytically
        funcs = [lambdify(alpha, eq, modules=['numpy']) for alpha, eq in zip(alpha_syms, eqs)]

        # solve the expressions, radians
        solved_alphas = [fsolve(func, 0) for func in funcs]

        # distance from base of linkage to center axis of focus mechanism, in
        ws = [self.R_AXIS + self.B_3 * math.cos(alpha) + self.B_4 * math.sin(alpha) for alpha in solved_alphas]

        # height between light rotation point and base of hexagon plate
        kappas = [math.sqrt(self.L_ARM ** 2 - (w - self.R_TOP) ** 2) + self.B_4 * math.cos(alpha) - self.B_3 * math.sin(alpha) for w, alpha in zip(ws, solved_alphas)]

        # target distance for focus mechanism to be at
        ds = [kappa - self.KAPPA_0 for kappa in kappas]

        return ds
    
    def calc_all(self):
        pitch, l_dists = self.calc_pitch()
        yaw = self.calc_yaw()
        focus = self.calc_focus(pitch, l_dists)

        return pitch, yaw, focus

    def connect(self):
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.settimeout(self.timeout)
        s.bind((self.my_ip, self.port))
        s.listen(1)
        self.conn, self.conn_addr = s.accept()

    def transmit(self):
        pitch, yaw, focus = self.calc_all()

        send_str = "p1:{:.2f}p2:{:.2f}p3:{:.2f}p4:{:.2f}" \
                   "y1:{:.2f}y2:{:.2f}y3:{:.2f}y4:{:.2f}" \
                   "f1:{:.2f}f2:{:.2f}f3:{:.2f}f4:{:.2f}" \
                   "b:{}\n".format(
                    pitch[0], pitch[1], pitch[2],
                    yaw[0], yaw[1], yaw[2],
                    focus[0], focus[1], focus[2],
                    self.b
                   )
        
        self.conn.sendall(bytes(send_str, 'utf-8'))

# Define our different screens
class LandingPage(Screen):
    pass

class HomePage(Screen):
    pass

class CalibrationPage(Screen):
    pass

class DefinePage(Screen):
    pass

class AdjustPage(Screen):
    pass

class TeachingPage(Screen):
    pass

class PageManager(ScreenManager):
    pass

class KivyCamera(Image):
    def __init__(self, fps=10, **kwargs):
        super(KivyCamera, self).__init__(**kwargs)
        # self.source = "images/body.jpg"

        Clock.schedule_interval(self.update, 1.0 / fps)

    def update(self, dt):
        ret, frame = KivyCamera.capture.read()
        if ret:
            # convert it to texture
            rot_flip = cv2.rotate(cv2.flip(frame, 0), cv2.ROTATE_90_CLOCKWISE)
            crop_dim_scaling = 1/3
            crop = rot_flip[int(rot_flip.shape[0]/2 - rot_flip.shape[0]*crop_dim_scaling/2):int(rot_flip.shape[0]/2 + rot_flip.shape[0]*crop_dim_scaling/2),
                int(rot_flip.shape[1]/2 - rot_flip.shape[1]*crop_dim_scaling/2):int(rot_flip.shape[1]/2 + rot_flip.shape[1]*crop_dim_scaling/2)]
            buf = crop.tobytes()

            # flip crop dimensions in size because orgiginal frame was rotated
            image_texture = Texture.create(
                size=(crop.shape[1], crop.shape[0]), colorfmt='bgr')
            image_texture.blit_buffer(buf, colorfmt='bgr', bufferfmt='ubyte')
            # display image from the texture
            self.texture = image_texture

# class TeachingCamera(Image):
#     def __init__(self, ip, fps=10, **kwargs):
#         super(KivyCamera, self).__init__(**kwargs)
#         # self.source = "images/body.jpg"
#         self.ip = "rtsp://zoilcam:zoilZOIL!@" + ip + "/stream1"
#         self.capture = cv2.VideoCapture(self.ip)
#         Clock.schedule_interval(self.update, 1.0 / fps)

#     def update(self, dt):
#         ret, frame = self.capture.read()
#         if ret:
#             # convert it to texture
#             rot_flip = cv2.rotate(cv2.flip(frame, 0), cv2.ROTATE_90_CLOCKWISE)
#             crop_dim_scaling = 1/3
#             crop = rot_flip[int(rot_flip.shape[0]/2 - rot_flip.shape[0]*crop_dim_scaling/2):int(rot_flip.shape[0]/2 + rot_flip.shape[0]*crop_dim_scaling/2),
#                 int(rot_flip.shape[1]/2 - rot_flip.shape[1]*crop_dim_scaling/2):int(rot_flip.shape[1]/2 + rot_flip.shape[1]*crop_dim_scaling/2)]
#             buf = crop.tobytes()

#             # flip crop dimensions in size because orgiginal frame was rotated
#             image_texture = Texture.create(
#                 size=(crop.shape[1], crop.shape[0]), colorfmt='bgr')
#             image_texture.blit_buffer(buf, colorfmt='bgr', bufferfmt='ubyte')
#             # display image from the texture
#             self.texture = image_texture

class CircleDrawingWidget(Widget):

    def __init__(self, **kwargs):
        super(CircleDrawingWidget, self).__init__(**kwargs)
        self.cent_x = 0
        self.cent_y = 0
        self.rad = 0

        # self.min_rad = 0
        # self.max_rad = self.width
        with self.canvas:
            self.circ = Line(circle=(self.cent_x, self.cent_y, self.rad))

    def shift(self, x_shift, y_shift):
        new_cent_x = self.cent_x + x_shift
        new_cent_y = self.cent_y + y_shift

        if (self.collide_point(new_cent_x + self.rad, new_cent_y) and
            self.collide_point(new_cent_x - self.rad, new_cent_y) and
            self.collide_point(new_cent_x, new_cent_y + self.rad) and
            self.collide_point(new_cent_x, new_cent_y - self.rad)):

            self.cent_x = new_cent_x
            self.cent_y = new_cent_y

            with self.canvas:
                self.canvas.remove(self.circ)
                Color(1, 0, 0, 1, mode="rgba")
                self.circ = Line(circle=(self.cent_x, self.cent_y, self.rad))
        
    def resize(self, rad_change):
        new_rad = self.rad + rad_change

        if (self.collide_point(self.cent_x + new_rad, self.cent_y) and
            self.collide_point(self.cent_x - new_rad, self.cent_y) and
            self.collide_point(self.cent_x, self.cent_y + new_rad) and
            self.collide_point(self.cent_x, self.cent_y - new_rad)):

            self.rad = new_rad

            with self.canvas:
                self.canvas.remove(self.circ)
                Color(1, 0, 0, 1, mode="rgba")
                self.circ = Line(circle=(self.cent_x, self.cent_y, self.rad))

    def on_touch_down(self, touch):
        if self.collide_point(touch.x, touch.y):
            self.cent_x = touch.x
            self.cent_y = touch.y
            self.rad = 0
            with self.canvas:
                self.canvas.remove(self.circ)
                self.circ = Line(circle=(self.cent_x, self.cent_y, self.rad))

    def on_touch_move(self, touch):
        
        new_rad = math.sqrt((self.cent_x - touch.x)**2 + (self.cent_y - touch.y)**2)

        if (self.collide_point(self.cent_x + new_rad, self.cent_y) and
            self.collide_point(self.cent_x - new_rad, self.cent_y) and
            self.collide_point(self.cent_x, self.cent_y + new_rad) and
            self.collide_point(self.cent_x, self.cent_y - new_rad)):
            
            self.rad = new_rad

            with self.canvas:
                self.canvas.remove(self.circ)
                Color(1, 0, 0, 1, mode="rgba")
                self.circ = Line(circle=(self.cent_x, self.cent_y, self.rad))

    def on_touch_up(self, touch):
        if self.collide_point(touch.x, touch.y):
            self.len_size = 6.75
            self.width_size = 2

            diff_x = (self.cent_x - self.center_x) * self.width_size / self.width
            diff_y = (self.cent_y - self.center_y) * self.len_size / self.height
            diff_rad = self.rad * self.width_size / self.width
            # print(self.ids.vid.norm_image_size)
            # print("full center: ", self.ids.vid.center_x, self.ids.vid.center_y)
            # print("circ center: ", self.cent_x, self.cent_y, self.rad)
            print("coords: ", diff_x, diff_y, diff_rad)

class RectDrawingWidget(Widget):
    def __init__(self, **kwargs):
        super(RectDrawingWidget, self).__init__(**kwargs)
        with self.canvas:
            self.rect = Line(rectangle=(0, 0, 0, 0))

    def on_touch_down(self, touch):
        self.corner1 = (touch.x, touch.y)
        self.corner2 = (touch.x, touch.y)

        if self.collide_point(touch.x, touch.y):
            with self.canvas:
                self.canvas.remove(self.rect)
                self.rect = Line(rectangle=(touch.x, touch.y, 0, 0))

    def on_touch_move(self, touch):
        self.corner2 = (touch.x, touch.y)

        if (self.collide_point(self.corner1[0], self.corner1[1]) and
            self.collide_point(self.corner2[0], self.corner2[1])):

            pos_x = min(self.corner1[0], self.corner2[0])
            pos_y = min(self.corner1[1], self.corner2[1])

            cal_height = abs(self.corner1[1] - self.corner2[1])
            cal_width = abs(self.corner1[0] - self.corner2[0])

            with self.canvas:
                self.canvas.remove(self.rect)
                Color(0, 1, 0, 1, mode="rgba")
                self.rect = Line(rectangle=(pos_x, pos_y, cal_width, cal_height))

    def on_touch_up(self, touch):
        if self.collide_point(touch.x, touch.y):
            self.len_size = 6.75
            self.width_size = 2

            pos_x = min(self.corner1[0], self.corner2[0])
            pos_y = min(self.corner1[1], self.corner2[1])

            cal_height = abs(self.corner1[1] - self.corner2[1])
            cal_width = abs(self.corner1[0] - self.corner2[0])

            # diff_x = (self.cent_x - self.center_x) * self.width_size / self.width
            # diff_y = (self.cent_y - self.center_y) * self.len_size / self.height
            # diff_rad = self.rad * self.width_size / self.width
            # print(self.ids.vid.norm_image_size)
            # print("full center: ", self.ids.vid.center_x, self.ids.vid.center_y)
            # print("circ center: ", self.cent_x, self.cent_y, self.rad)
            print("coords: ", pos_x, pos_y, cal_height, cal_width)


class ZoilApp(MDApp):
    title = "ZoilDef App"

    my_config = Config()
    
    KivyCamera.ip = "rtsp://zoilcam:zoilZOIL!@" + my_config.cam_ip + "/stream1"
    KivyCamera.capture = cv2.VideoCapture(KivyCamera.ip)

    def build(self):
        # layout =  ZoilDefLayout()
        layout = Builder.load_file("zoildef.kv")
        return layout

    def on_pause(self):
        return True

if __name__ == '__main__':
    Window.clearcolor = (0, 0, 0, 1)
    ZoilApp().run()