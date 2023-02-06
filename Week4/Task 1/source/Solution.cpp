#include "Common.h"

using namespace std;

// Этот файл сдаётся на проверку
// Здесь напишите реализацию необходимых классов-потомков `IShape`
class Ellipse : public IShape {
public:
    std::unique_ptr<IShape> Clone() const override {
        auto new_shape = make_unique<Ellipse>();
        new_shape->SetPosition(m_position);
        new_shape->SetSize(m_size);
        new_shape->SetTexture(m_texture);

        return new_shape;
    }

    void SetPosition(Point position) override {
        m_position = position;
    }

    Point GetPosition() const override {
        return m_position;
    }

    void SetSize(Size size) override {
        m_size = size;
    }

    Size GetSize() const override {
        return m_size;
    }

    void SetTexture(std::shared_ptr<ITexture> texture) override {
        m_texture = texture;
    }

    ITexture* GetTexture() const override {
        return m_texture.get();
    }

    // Рисует фигуру на указанном изображении
    void Draw(Image& image) const override {
        int x_max = m_size.width + m_position.x;
        int y_max = m_size.height + m_position.y;
        int y_offset = m_position.y;
        int x_offset = m_position.x;

        for (int y = m_position.y; y < y_max && y < image.size(); y++) {
            for (int x = m_position.x; x < x_max && x < image[y].size(); x++) {
                if (!IsPointInEllipse({.x = x - x_offset, .y = y - y_offset}, m_size)) {
                    continue;
                }

                if (m_texture) {
                    auto size = m_texture->GetSize();
                    if (x - x_offset < size.width && y - y_offset < size.height) {
                        auto tex_img = m_texture->GetImage();
                        image[y][x] = tex_img[y - y_offset][x - x_offset];
                    }
                    else {
                        image[y][x] = '.';
                    }
                }
                else {
                    image[y][x] = '.';
                }
            }
        }
    }

private:
    shared_ptr<ITexture> m_texture;
    Point m_position;
    Size m_size;
};

class Rectangle : public IShape {
public:
    std::unique_ptr<IShape> Clone() const override {
        auto new_shape = make_unique<Rectangle>();
        new_shape->SetPosition(m_position);
        new_shape->SetSize(m_size);
        new_shape->SetTexture(m_texture);

        return new_shape;
    }

    void SetPosition(Point position) override {
        m_position = position;
    }

    Point GetPosition() const override {
        return m_position;
    }

    void SetSize(Size size) override {
        m_size = size;
    }

    Size GetSize() const override {
        return m_size;
    }

    void SetTexture(std::shared_ptr<ITexture> texture) override {
        m_texture = texture;
    }

    ITexture* GetTexture() const override {
        return m_texture.get();
    }

    // Рисует фигуру на указанном изображении
    void Draw(Image& image) const override {
        int x_max = m_size.width + m_position.x;
        int y_max = m_size.height + m_position.y;
        int y_offset = m_position.y;
        int x_offset = m_position.x;

        for (int y = m_position.y; y < y_max && y < image.size(); y++) {
            for (int x = m_position.x; x < x_max && x < image[y].size(); x++) {
                if (m_texture) {
                    auto size = m_texture->GetSize();
                    if (x - x_offset < size.width && y - y_offset < size.height) {
                        auto tex_img = m_texture->GetImage();
                        image[y][x] = tex_img[y - y_offset][x - x_offset];
                    }
                    else {
                        image[y][x] = '.';
                    }
                }
                else {
                    image[y][x] = '.';
                }
            }
        }
    }

private:
    shared_ptr<ITexture> m_texture = nullptr;
    Point m_position;
    Size m_size;
};


// Напишите реализацию функции
unique_ptr<IShape> MakeShape(ShapeType shape_type) {
    switch (shape_type) {
        case ShapeType::Ellipse:
            return make_unique<Ellipse>();
            break;
        case ShapeType::Rectangle:
            return make_unique<Rectangle>();
    }

    return nullptr;
}