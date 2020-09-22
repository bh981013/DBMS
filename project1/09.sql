select t.name, avg(c.level) as avr_level
from CatchedPokemon c, Trainer t
where t.id = c.owner_id and
      c.owner_id in (select r.id
                     from gym g, trainer r
                     where g.leader_id = r.id)
group by t.name
order by t.name;