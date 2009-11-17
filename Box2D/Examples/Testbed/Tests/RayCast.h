/*
* Copyright (c) 2006-2009 Erin Catto http://www.gphysics.com
*
* This software is provided 'as-is', without any express or implied
* warranty.  In no event will the authors be held liable for any damages
* arising from the use of this software.
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
* 1. The origin of this software must not be misrepresented; you must not
* claim that you wrote the original software. If you use this software
* in a product, an acknowledgment in the product documentation would be
* appreciated but is not required.
* 2. Altered source versions must be plainly marked as such, and must not be
* misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
*/

#ifndef RAY_CAST_H
#define RAY_CAST_H

class RayCastCallback : public b2RayCastCallback
{
public:
	RayCastCallback()
	{
		m_fixture = NULL;
	}

	float32 ReportFixture(	b2Fixture* fixture, const b2Vec2& point,
		const b2Vec2& normal, float32 fraction)
	{
		m_fixture = fixture;
		m_point = point;
		m_normal = normal;

		return fraction;
	}

	b2Fixture* m_fixture;
	b2Vec2 m_point;
	b2Vec2 m_normal;
};

class RayCast : public Test
{
public:

	enum
	{
		e_maxBodies = 256,
	};

	RayCast()
	{
		// Ground body
		{
			b2BodyDef bd;
			b2Body* ground = m_world->CreateBody(&bd);

			b2PolygonShape shape;
			shape.SetAsEdge(b2Vec2(-40.0f, 0.0f), b2Vec2(40.0f, 0.0f));
			ground->CreateFixture(&shape);
		}

		{
			b2Vec2 vertices[3];
			vertices[0].Set(-0.5f, 0.0f);
			vertices[1].Set(0.5f, 0.0f);
			vertices[2].Set(0.0f, 1.5f);
			m_polygons[0].Set(vertices, 3);
		}

		{
			b2Vec2 vertices[3];
			vertices[0].Set(-0.1f, 0.0f);
			vertices[1].Set(0.1f, 0.0f);
			vertices[2].Set(0.0f, 1.5f);
			m_polygons[1].Set(vertices, 3);
		}

		{
			float32 w = 1.0f;
			float32 b = w / (2.0f + sqrtf(2.0f));
			float32 s = sqrtf(2.0f) * b;

			b2Vec2 vertices[8];
			vertices[0].Set(0.5f * s, 0.0f);
			vertices[1].Set(0.5f * w, b);
			vertices[2].Set(0.5f * w, b + s);
			vertices[3].Set(0.5f * s, w);
			vertices[4].Set(-0.5f * s, w);
			vertices[5].Set(-0.5f * w, b + s);
			vertices[6].Set(-0.5f * w, b);
			vertices[7].Set(-0.5f * s, 0.0f);

			m_polygons[2].Set(vertices, 8);
		}

		{
			m_polygons[3].SetAsBox(0.5f, 0.5f);
		}

		{
			m_circle.m_radius = 0.5f;
		}

		m_bodyIndex = 0;
		memset(m_bodies, 0, sizeof(m_bodies));

		m_angle = 0.0f;
	}

	void Create(int32 index)
	{
		if (m_bodies[m_bodyIndex] != NULL)
		{
			m_world->DestroyBody(m_bodies[m_bodyIndex]);
			m_bodies[m_bodyIndex] = NULL;
		}

		b2BodyDef bd;

		float32 x = RandomFloat(-10.0f, 10.0f);
		float32 y = RandomFloat(0.0f, 20.0f);
		bd.position.Set(x, y);
		bd.angle = RandomFloat(-b2_pi, b2_pi);

		if (index == 4)
		{
			bd.angularDamping = 0.02f;
		}

		m_bodies[m_bodyIndex] = m_world->CreateBody(&bd);

		if (index < 4)
		{
			b2FixtureDef fd;
			fd.shape = m_polygons + index;
			fd.friction = 0.3f;
			m_bodies[m_bodyIndex]->CreateFixture(&fd);
		}
		else
		{
			b2FixtureDef fd;
			fd.shape = &m_circle;
			fd.friction = 0.3f;

			m_bodies[m_bodyIndex]->CreateFixture(&fd);
		}

		m_bodyIndex = (m_bodyIndex + 1) % e_maxBodies;
	}

	void DestroyBody()
	{
		for (int32 i = 0; i < e_maxBodies; ++i)
		{
			if (m_bodies[i] != NULL)
			{
				m_world->DestroyBody(m_bodies[i]);
				m_bodies[i] = NULL;
				return;
			}
		}
	}

	void Keyboard(unsigned char key)
	{
		switch (key)
		{
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
			Create(key - '1');
			break;

		case 'd':
			DestroyBody();
			break;
		}
	}

	void Step(Settings* settings)
	{
		Test::Step(settings);
		m_debugDraw.DrawString(5, m_textLine, "Press 1-5 to drop stuff");
		m_textLine += 15;

		float32 L = 11.0f;
		b2Vec2 point1(0.0f, 10.0f);
		b2Vec2 d(L * cosf(m_angle), L * sinf(m_angle));
		b2Vec2 point2 = point1 + d;

		RayCastCallback callback;

		m_world->RayCast(&callback, point1, point2);

		if (callback.m_fixture)
		{
			m_debugDraw.DrawPoint(callback.m_point, 5.0f, b2Color(0.4f, 0.9f, 0.4f));

			m_debugDraw.DrawSegment(point1, callback.m_point, b2Color(0.8f, 0.8f, 0.8f));

			b2Vec2 head = callback.m_point + 0.5f * callback.m_normal;
			m_debugDraw.DrawSegment(callback.m_point, head, b2Color(0.9f, 0.9f, 0.4f));
		}
		else
		{
			m_debugDraw.DrawSegment(point1, point2, b2Color(0.8f, 0.8f, 0.8f));
		}

		m_angle += 0.25f * b2_pi / 180.0f;
	}

	static Test* Create()
	{
		return new RayCast;
	}

	int32 m_bodyIndex;
	b2Body* m_bodies[e_maxBodies];
	b2PolygonShape m_polygons[4];
	b2CircleShape m_circle;

	float32 m_angle;
};

#endif